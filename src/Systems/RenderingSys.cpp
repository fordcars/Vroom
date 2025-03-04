#include "RenderingSys.hpp"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp> // For lookAt()

#include "Constants.hpp"
#include "Entities/EntityFilter.hpp"
#include "Log.hpp"
#include "ResourceSys/Obj/ObjResource.hpp"
#include "ResourceSys/ResourceSys.hpp"

// Static
RenderingSys& RenderingSys::get() {
    static std::unique_ptr<RenderingSys> instance = std::make_unique<RenderingSys>();
    return *instance;
}

RenderingSys::~RenderingSys() { SDL_GL_DeleteContext(mContext); }

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
    SDL_GL_SetSwapInterval(1);

    initGL(window);
    return true;
}

void RenderingSys::clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void RenderingSys::render(SDL_Window* window) {
    mCurrentTime = SDL_GetTicks();
    EntityFilter<PositionComp, RenderableComp, std::optional<AnimationComp>> filter;
    for(const auto& [position, renderable, animation] : filter) {
        renderEntity(position, renderable, animation);
    }

    // Check for gl error
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        Log::glError(error) << "Encountered an OpenGL error while rendering!";
    }
    SDL_GL_SwapWindow(window);
}

void RenderingSys::initGL(SDL_Window* window) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint vertexArrayID;
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glClearColor(Constants::BG_COLOR.r, Constants::BG_COLOR.g, Constants::BG_COLOR.b,
                 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Accept the fragment closer to the camera

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_MULTISAMPLE); // Enable anti-aliasing (SDL attributes)
}

void RenderingSys::renderEntity(
    const PositionComp& position, const RenderableComp& renderable,
    std::optional<std::reference_wrapper<AnimationComp>> animation) {
    const ShaderResource& shader = *renderable.shader;
    const CameraEntity& camera = CameraEntity::instances[0];
    const GLsizei stride = sizeof(ObjResource::Vertex);

    GLint skeletonTransformUnformBlock = -1;
    bool hasAnimation =
        animation && (skeletonTransformUnformBlock = renderable.shader->findUniformBlock(
                          "SkeletonTransformBlock")) != -1;

    glUseProgram(shader.getId());
    glBindBuffer(GL_ARRAY_BUFFER, renderable.objectResource->vertexBuffer.getId());

    // Per object uniforms
    glUniform1ui(shader.findUniform("time"), mCurrentTime);
    if(renderable.shader->findUniformBlock("ObjMaterialsBlock") != -1) {
        glBindBufferBase(GL_UNIFORM_BUFFER,
                         renderable.shader->findUniformBlock("ObjMaterialsBlock"),
                         renderable.objectResource->materialUniformBuffer.getId());
    }

    if(hasAnimation) {
        glEnableVertexAttribArray(3); // Bone IDs
        glEnableVertexAttribArray(4); // Bone weights

        // Joint indices
        glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, stride,
                               (void*)offsetof(ObjResource::Vertex, joints));

        // Weights
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride,
                              (void*)offsetof(ObjResource::Vertex, weights));
    }

    // Enable vertex attributes
    glEnableVertexAttribArray(0); // Position
    glEnableVertexAttribArray(1); // Normal
    glEnableVertexAttribArray(2); // Material ID

    // Position (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(ObjResource::Vertex, position));

    // Normal (vec3)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(ObjResource::Vertex, normal));

    // Material ID
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, stride,
                           (void*)offsetof(ObjResource::Vertex, materialId));

    // Render all meshes
    for(const auto& mesh : renderable.meshes) {
        // Per mesh uniforms
        glm::mat4 modelMatrix = getModelMatrix(position) * mesh->transform;
        glm::mat4 viewMatrix = getViewMatrix(camera);
        glm::mat4 projectionMatrix = getProjectionMatrix(camera);

        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
        glm::mat4 MVP = projectionMatrix * modelViewMatrix;

        glUniformMatrix4fv(shader.findUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(shader.findUniform("modelMatrix"), 1, GL_FALSE,
                           &modelMatrix[0][0]);
        glUniformMatrix4fv(shader.findUniform("viewMatrix"), 1, GL_FALSE,
                           &viewMatrix[0][0]);
        glUniformMatrix4fv(shader.findUniform("projectionMatrix"), 1, GL_FALSE,
                           &projectionMatrix[0][0]);
        glUniformMatrix4fv(shader.findUniform("normalMatrix"), 1, GL_FALSE,
                           &normalMatrix[0][0]);

        if(hasAnimation && mesh->skeleton) {
            glBindBufferBase(GL_UNIFORM_BUFFER, skeletonTransformUnformBlock,
                             mesh->skeleton->getTransformBuffer().getId());
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer.getId());
        glDrawElements(GL_TRIANGLES, // Mode
                       mesh->indexBuffer.getCount(),
                       GL_UNSIGNED_INT, // Type
                       (void*)0         // Element array buffer offset
        );
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
}

glm::mat4 RenderingSys::getModelMatrix(const PositionComp& position) {
    return glm::translate(glm::mat4(1.0f), position.coords);
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