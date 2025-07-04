#include "RenderingSys.hpp"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp> // For lookAt()

#include "Components/PhysicsComp.hpp"
#include "Components/PositionComp.hpp"
#include "Constants.hpp"
#include "Entities/EntityFilter.hpp"
#include "Log.hpp"
#include "ResourceSys/Obj/Animation/Skin.hpp"
#include "ResourceSys/Obj/GPUBuffer.hpp"
#include "ResourceSys/Obj/ObjResource.hpp"
#include "ResourceSys/ResourceSys.hpp"
#include "UISys.hpp"

// Static
RenderingSys& RenderingSys::get() {
    static std::unique_ptr<RenderingSys> instance = std::make_unique<RenderingSys>();
    return *instance;
}

RenderingSys::~RenderingSys() {
    SDL_GL_DeleteContext(mContext);
    glDeleteTextures(GBufferTexture::COUNT, mDeferredTextures);
}

bool RenderingSys::init(SDL_Window* window) {
    Log::info() << "Initializing graphics...";

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, Constants::OPENGL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, Constants::OPENGL_MINOR_VERSION);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create context
    mContext = SDL_GL_CreateContext(window);
    if(!mContext) {
        Log::sdlError() << "Unable to create OpenGL context! This game requires OpenGL "
                        << Constants::OPENGL_MAJOR_VERSION << "."
                        << Constants::OPENGL_MINOR_VERSION
                        << ", make sure your system supports it!";
        return false;
    }

    // Load GL functions
    if(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0) {
        Log::error() << "Failed to load OpenGL!";
        return false;
    }

    Log::info() << "OpenGL: " << (const char*)glGetString(GL_VERSION);
    SDL_GL_SetSwapInterval(Constants::ENABLE_VSYNC ? 1 : 0);

    initGL(window);
    UISys::get().init(window, mContext);

    return true;
}

void RenderingSys::clear() {
    // Must clear deferred framebuffers to black, since we are using additive blending
    glBindFramebuffer(GL_FRAMEBUFFER, mDeferredFramebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, mPostProcessFramebuffer);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderingSys::render(SDL_Window* window) {
    std::vector<std::tuple<PositionComp*, RenderableComp*>> forwardShadedEntities;

    const CameraEntity& camera = CameraEntity::instances[0];
    glm::mat4 viewMatrix = getViewMatrix(camera);
    glm::mat4 projectionMatrix = getProjectionMatrix(camera);
    mCurrentTime = SDL_GetTicks();

    // First pass: render entities to GBuffer
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, mDeferredFramebuffer);

    EntityFilter<PositionComp, RenderableComp> renderableFilter;
    for(const auto& [position, renderable] : renderableFilter) {
        if(renderable.shadingType == RenderableComp::ShadingType::ForwardShaded) {
            forwardShadedEntities.emplace_back(&position, &renderable);
        } else {
            renderRenderable(viewMatrix, projectionMatrix, position, renderable);
        }
    }

    // Second pass: render lights using GBuffer
    GLuint lightTargetFramebuffer = mPostProcessShader ? mPostProcessFramebuffer : 0;
    cloneDepthBuffer(mDeferredFramebuffer,
                     lightTargetFramebuffer); // Clone depth buffer to front, used later
    glDepthMask(GL_FALSE);                    // Disable writing to depth buffer
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); // Enable blending to add each light source
    glBindFramebuffer(GL_FRAMEBUFFER, lightTargetFramebuffer);

    EntityFilter<PositionComp, LightComp> lightFilter;
    for(const auto& [position, light] : lightFilter) {
        renderLight(viewMatrix, position, light);
    }

    // Third pass: render forward shaded entities
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    for(const auto& [position, renderable] : forwardShadedEntities) {
        renderRenderable(viewMatrix, projectionMatrix, *position, *renderable);
    }

    // Fourth pass: post-process rendering if shader is present
    if(mPostProcessShader) {
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        renderPostProcessing();
    }

    // Render debug stuff if present, same setup as forward shaded
    const auto& shader = ResourceSys::get().getShaderResource("basic");
    for(const auto& [drawMode, shapes] : mDebugShapes) {
        for(const auto& shape : shapes) {
            renderDebugShape(*shader, viewMatrix, projectionMatrix, shape, drawMode);
        }
    }
    mDebugShapes.clear();

    // Check for gl error
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        Log::glError(error) << "Encountered an OpenGL error while rendering!";
    }

    // Render UI and swap window
    UISys::get().render();
    SDL_GL_SwapWindow(window); // Waits for VSync if enabled
}

void RenderingSys::setPostProcessShader(ShaderResource::CPtr shader) {
    mPostProcessShader = std::move(shader);
}

void RenderingSys::addDebugShape(const std::vector<glm::vec3>& points,
                                 const std::vector<glm::vec3>& colors, GLenum drawMode) {
    // Resize colors if not same size
    std::vector<glm::vec3> resizedColors(points.size(),
                                         glm::vec3(0.0f, 1.0f, 0.0f)); // Default to green
    std::copy(colors.begin(), colors.begin() + std::min(points.size(), colors.size()),
              resizedColors.begin());

    mDebugShapes[drawMode].emplace_back(DebugShape{points, resizedColors});
}

void RenderingSys::initGL(SDL_Window* window) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    mScreenSize.x = width;
    mScreenSize.y = height;
    glViewport(0, 0, width, height);

    GLuint vertexArrayID;
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Accept the fragment closer to the camera

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_MULTISAMPLE); // Enable anti-aliasing (SDL attributes)

    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    initDeferredRendering();
    initPostProcessRendering();
}

void RenderingSys::initDeferredRendering() {
    static constexpr std::pair<GBufferTexture, GLenum> TEXTURE_FORMATS[] = {
        {GBufferTexture::Position, GL_RGB32F}, {GBufferTexture::Normal, GL_RGB32F},
        {GBufferTexture::Albedo, GL_RGB8},     {GBufferTexture::Metallic, GL_RED},
        {GBufferTexture::Roughness, GL_RED},
    };
    static_assert(std::size(TEXTURE_FORMATS) == GBufferTexture::COUNT);

    // Check system compatibility
    if(GBufferTexture::COUNT > GL_MAX_COLOR_ATTACHMENTS) {
        Log::error() << "GBufferTexture::COUNT exceeds GL_MAX_COLOR_ATTACHMENTS!";
    }

    // Create framebuffer
    glGenFramebuffers(1, &mDeferredFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mDeferredFramebuffer);

    // Add depth buffer
    glGenRenderbuffers(1, &mDeferredDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mDeferredDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mScreenSize.x,
                          mScreenSize.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              mDeferredDepthBuffer);

    // Generate textures
    glGenTextures(GBufferTexture::COUNT, mDeferredTextures);
    for(size_t i = 0; i < GBufferTexture::COUNT; ++i) {
        auto&& [texture, format] = TEXTURE_FORMATS[i];
        glBindTexture(GL_TEXTURE_2D, mDeferredTextures[texture]);
        glTexImage2D(GL_TEXTURE_2D, 0, format, mScreenSize.x, mScreenSize.y, 0, GL_RED,
                     GL_FLOAT, nullptr);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + texture,
                             mDeferredTextures[texture], 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    // Set draw buffers
    GLenum drawBuffers[GBufferTexture::COUNT];
    for(std::size_t i = 0; i < GBufferTexture::COUNT; i++) {
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glDrawBuffers(GBufferTexture::COUNT, drawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Log::error() << "Could not initialize deferred rendering framebuffer!";
    }

    // Check for gl error
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        Log::glError(error)
            << "Encountered an OpenGL error while initializing deferred rendering!";
    }
}

void RenderingSys::initPostProcessRendering() {
    // Create framebuffer
    glGenFramebuffers(1, &mPostProcessFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mPostProcessFramebuffer);

    // Add depth buffer
    glGenRenderbuffers(1, &mPostProcessDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mPostProcessDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mScreenSize.x,
                          mScreenSize.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              mPostProcessDepthBuffer);

    // Generate texture
    glGenTextures(1, &mPostProcessTexture);
    glBindTexture(GL_TEXTURE_2D, mPostProcessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, mScreenSize.x, mScreenSize.y, 0, GL_RED,
                 GL_FLOAT, nullptr);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mPostProcessTexture, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set draw buffer
    GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Log::error() << "Could not initialize post-process rendering framebuffer!";
    }

    // Check for gl error
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        Log::glError(error)
            << "Encountered an OpenGL error while initializing post-process rendering!";
    }
}

void RenderingSys::renderRenderable(const glm::mat4& viewMatrix,
                                    const glm::mat4& projectionMatrix,
                                    const PositionComp& position,
                                    const RenderableComp& renderable) {
    constexpr unsigned POSITION_ATTRIB = 0;
    constexpr unsigned NORMAL_ATTRIB = 1;
    constexpr unsigned TEXCOORD_ATTRIB = 2;
    constexpr unsigned MATERIAL_ATTRIB = 3;
    constexpr unsigned BONE_ID_ATTRIB = 4;
    constexpr unsigned WEIGHT_ATTRIB = 5;

    auto setTexture = [](std::size_t samplerUniformLocation,
                         std::size_t hasSamplerUniformLocation, const ObjTexture* texture,
                         GLuint textureUnit) {
        if(texture) {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, texture->image->textureId);
            glBindSampler(textureUnit, texture->samplerId);
            glUniform1i(samplerUniformLocation, textureUnit);
            glUniform1i(hasSamplerUniformLocation, 1);

        } else {
            glUniform1i(samplerUniformLocation, 0);
            glUniform1i(hasSamplerUniformLocation, 0);
        }
    };

    if(!renderable.objectResource || !renderable.shader) {
        return;
    }

    using namespace Constants;
    const ShaderResource& shader = *renderable.shader;
    const GLsizei stride = sizeof(ObjResource::Vertex);

    GLint skinTransformUnformBlock = -1;
    GLint isSkinnedUniform = -1;
    bool isSkinnedShader = (skinTransformUnformBlock = renderable.shader->getUniformBlock(
                                UniformBlockName::get<"SkinTransformBlock">())) != -1 &&
                           (isSkinnedUniform = renderable.shader->getUniform(
                                UniformName::get<"isSkinned">())) != -1;

    glUseProgram(shader.getId());
    glBindBuffer(GL_ARRAY_BUFFER, renderable.objectResource->vertexBuffer.getId());

    // Per object uniforms
    glUniform1ui(shader.getUniform(UniformName::get<"time">()), mCurrentTime);
    GLint materialsBlock = -1;
    if((materialsBlock = renderable.shader->getUniformBlock(
            UniformBlockName::get<"ObjMaterialsBlock">())) != -1) {
        glBindBufferBase(GL_UNIFORM_BUFFER, materialsBlock,
                         renderable.objectResource->materialUniformBuffer.getId());
    }

    if(isSkinnedShader) {
        glEnableVertexAttribArray(BONE_ID_ATTRIB);
        glEnableVertexAttribArray(WEIGHT_ATTRIB);

        // Joint indices
        glVertexAttribIPointer(BONE_ID_ATTRIB, 4, GL_UNSIGNED_INT, stride,
                               (void*)offsetof(ObjResource::Vertex, joints));

        // Weights
        glVertexAttribPointer(WEIGHT_ATTRIB, 4, GL_FLOAT, GL_FALSE, stride,
                              (void*)offsetof(ObjResource::Vertex, weights));
    }

    // Enable vertex attributes
    glEnableVertexAttribArray(POSITION_ATTRIB);
    glEnableVertexAttribArray(NORMAL_ATTRIB);
    glEnableVertexAttribArray(TEXCOORD_ATTRIB);
    glEnableVertexAttribArray(MATERIAL_ATTRIB);

    // Position (vec3)
    glVertexAttribPointer(POSITION_ATTRIB, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(ObjResource::Vertex, position));

    // Normal (vec3)
    glVertexAttribPointer(NORMAL_ATTRIB, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(ObjResource::Vertex, normal));

    // Texcoord (vec2)
    glVertexAttribPointer(TEXCOORD_ATTRIB, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(ObjResource::Vertex, texcoord));

    // Material ID
    glVertexAttribIPointer(MATERIAL_ATTRIB, 1, GL_UNSIGNED_INT, stride,
                           (void*)offsetof(ObjResource::Vertex, materialId));

    // Render all meshes
    for(const auto& mesh : renderable.objectResource->objMeshes) {
        // Per mesh uniforms
        glm::mat4 modelMatrix = position.getTransform() * mesh->transform;

        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
        glm::mat4 MVP = projectionMatrix * modelViewMatrix;

        glUniformMatrix4fv(shader.getUniform(UniformName::get<"MVP">()), 1, GL_FALSE,
                           &MVP[0][0]);
        glUniformMatrix4fv(shader.getUniform(UniformName::get<"modelMatrix">()), 1,
                           GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(shader.getUniform(UniformName::get<"viewMatrix">()), 1,
                           GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(shader.getUniform(UniformName::get<"projectionMatrix">()), 1,
                           GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(shader.getUniform(UniformName::get<"normalMatrix">()), 1,
                           GL_FALSE, &normalMatrix[0][0]);

        // Set textures if present
        int textureUnit = 0;
        setTexture(shader.getUniform(UniformName::get<"baseColorTex">()),
                   shader.getUniform(UniformName::get<"hasBaseColorTex">()),
                   mesh->baseColorTexture.get(), textureUnit++);
        setTexture(shader.getUniform(UniformName::get<"normalTex">()),
                   shader.getUniform(UniformName::get<"hasNormalTex">()),
                   mesh->normalTexture.get(), textureUnit++);
        setTexture(shader.getUniform(UniformName::get<"metallicRoughnessTex">()),
                   shader.getUniform(UniformName::get<"hasMetallicRoughnessTex">()),
                   mesh->metallicRoughnessTexture.get(), textureUnit++);
        setTexture(shader.getUniform(UniformName::get<"emissiveTex">()),
                   shader.getUniform(UniformName::get<"hasEmissiveTex">()),
                   mesh->emissiveTexture.get(), textureUnit++);

        glUniform1f(shader.getUniform(UniformName::get<"normalScale">()),
                    mesh->normalScale);

        if(isSkinnedShader) {
            if(mesh->skin) {
                glUniform1i(isSkinnedUniform, 1);
                glBindBufferBase(GL_UNIFORM_BUFFER, skinTransformUnformBlock,
                                 mesh->skin->getTransformBuffer().getId());
            } else {
                glUniform1i(isSkinnedUniform, 0);
            }
        }

        // Draw
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer.getId());
        glDrawElements(GL_TRIANGLES, // Mode
                       mesh->indexBuffer.getCount(),
                       GL_UNSIGNED_INT, // Type
                       (void*)0         // Element array buffer offset
        );
    }

    glDisableVertexAttribArray(POSITION_ATTRIB);
    glDisableVertexAttribArray(NORMAL_ATTRIB);
    glDisableVertexAttribArray(TEXCOORD_ATTRIB);
    glDisableVertexAttribArray(MATERIAL_ATTRIB);
    glDisableVertexAttribArray(BONE_ID_ATTRIB);
    glDisableVertexAttribArray(WEIGHT_ATTRIB);
}

void RenderingSys::renderLight(const glm::mat4& viewMatrix, const PositionComp& position,
                               const LightComp& light) {
    if(!light.shader) {
        return;
    }

    using namespace Constants;
    const ShaderResource& shader = *light.shader;
    const GLsizei stride = sizeof(LightComp::Vertex);

    glUseProgram(shader.getId());

    // Set uniforms
    glUniformMatrix4fv(shader.getUniform(UniformName::get<"viewMatrix">()), 1, GL_FALSE,
                       &viewMatrix[0][0]);
    glUniform3f(shader.getUniform(UniformName::get<"lightPos_worldspace">()),
                position.coords.x, position.coords.y, position.coords.z);
    glUniform3f(shader.getUniform(UniformName::get<"lightDiffuseColor">()),
                light.diffuse.r, light.diffuse.g, light.diffuse.b);
    glUniform3f(shader.getUniform(UniformName::get<"lightSpecularColor">()),
                light.specular.r, light.specular.g, light.specular.b);
    glUniform1f(shader.getUniform(UniformName::get<"lightIntensity">()), light.intensity);

    int textureUnit = 0;
    glUniform1i(shader.getUniform(UniformName::get<"positionTex">()), textureUnit++);
    glUniform1i(shader.getUniform(UniformName::get<"normalTex">()), textureUnit++);
    glUniform1i(shader.getUniform(UniformName::get<"albedoTex">()), textureUnit++);
    glUniform1i(shader.getUniform(UniformName::get<"metallicTex">()), textureUnit++);
    glUniform1i(shader.getUniform(UniformName::get<"roughnessTex">()), textureUnit++);

    // Set textures for samplers
    for(std::size_t i = 0; i < GBufferTexture::COUNT; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mDeferredTextures[i]);
        glBindSampler(i, 0);
    }

    // Attributes
    glBindBuffer(GL_ARRAY_BUFFER, light.vertexBuffer.getId());
    glEnableVertexAttribArray(0); // Positions
    glEnableVertexAttribArray(1); // Texcoords

    // Note at this point, we are probably drawing a simple, full screen quad
    // Positions (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(LightComp::Vertex, position));

    // UVs (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(LightComp::Vertex, texcoord));

    // Draw
    glDrawArrays(GL_TRIANGLES,     // Mode
                 0,                // Start
                 light.vertexCount // Count
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void RenderingSys::renderPostProcessing() {
    struct Vertex {
        glm::vec3 position;
        glm::vec2 texcoord;
    };
    static GPUBuffer quadBuffer(GL_ARRAY_BUFFER, std::vector<Vertex>{
                                                     {{-1, -1, 0}, {0, 0}},
                                                     {{1, -1, 0}, {1, 0}},
                                                     {{1, 1, 0}, {1, 1}},

                                                     {{-1, -1, 0}, {0, 0}},
                                                     {{1, 1, 0}, {1, 1}},
                                                     {{-1, 1, 0}, {0, 1}},
                                                 });
    static std::size_t quadVertexCount = quadBuffer.getCount();

    using namespace Constants;
    const ShaderResource& shader = *mPostProcessShader;
    const GLsizei stride = sizeof(Vertex);

    glUseProgram(shader.getId());
    glUniform1i(shader.getUniform(UniformName::get<"colorTex">()), 0);

    // Set textures for samplers
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mPostProcessTexture);
    glBindSampler(0, 0);

    // Attributes
    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.getId());
    glEnableVertexAttribArray(0); // Positions
    glEnableVertexAttribArray(1); // Texcoords

    // Note at this point, we are probably drawing a simple, full screen quad
    // Positions (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(Vertex, position));

    // UVs (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(Vertex, texcoord));

    // Draw
    glDrawArrays(GL_TRIANGLES,   // Mode
                 0,              // Start
                 quadVertexCount // Count
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void RenderingSys::renderDebugShape(const ShaderResource& shader,
                                    const glm::mat4& viewMatrix,
                                    const glm::mat4& projectionMatrix,
                                    const DebugShape& shape, GLenum drawMode) {
    if(shape.points.empty()) {
        return;
    }

    using namespace Constants;
    glUseProgram(shader.getId());

    // Set uniforms
    glm::mat4 MVP = projectionMatrix * viewMatrix; // No model matrix

    glUniformMatrix4fv(shader.getUniform(UniformName::get<"MVP">()), 1, GL_FALSE,
                       &MVP[0][0]);

    // Attributes, create buffers on the spot
    GPUBuffer posBuf(GL_ARRAY_BUFFER, shape.points, GL_STATIC_DRAW);
    GPUBuffer colorBuf(GL_ARRAY_BUFFER, shape.colors, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Positions (vec3)
    glBindBuffer(GL_ARRAY_BUFFER, posBuf.getId());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Colors (vec3)
    glBindBuffer(GL_ARRAY_BUFFER, colorBuf.getId());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Draw
    glDrawArrays(drawMode,           // Mode
                 0,                  // Start
                 shape.points.size() // Count
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void RenderingSys::cloneDepthBuffer(GLuint source, GLuint dest) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
    glBlitFramebuffer(0, 0, mScreenSize.x, mScreenSize.y, 0, 0, mScreenSize.x,
                      mScreenSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

glm::mat4 RenderingSys::getViewMatrix(const CameraEntity& camera) {
    glm::vec3 position = camera.get<PositionComp>().coords;
    const CameraInfoComp& info = camera.get<CameraInfoComp>();

    glm::vec3 vec3Direction;
    if(info.direction.w == 0) {
        // Direction is a vector
        vec3Direction = glm::vec3(info.direction) + position;
    } else {
        // Direction is a point (position)
        vec3Direction = glm::vec3(info.direction);
    }

    return glm::lookAt(position, vec3Direction, info.upVector);
}

glm::mat4 RenderingSys::getProjectionMatrix(const CameraEntity& camera) {
    glm::vec3 position = camera.get<PositionComp>().coords;
    const CameraInfoComp& info = camera.get<CameraInfoComp>();
    float vertFOX = 2 * atan(tan(info.horizFOV / 2) / info.aspectRatio);

    return glm::perspective(vertFOX, info.aspectRatio, info.nearClippingPlane,
                            info.farClippingPlane);
}