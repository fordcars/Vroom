#pragma once
#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <optional>

#include "Components/LightComp.hpp"
#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entities/CameraEntity.hpp"

class ShaderResource;
class RenderingSys {
public:
    static RenderingSys& get();
    RenderingSys() = default;
    ~RenderingSys();

    bool init(SDL_Window* window);
    void clear();
    void render(SDL_Window* window);
    void setPostProcessShader(ShaderResource::CPtr shader);

    void addDebugShape(const std::vector<glm::vec3>& points,
                       const std::vector<glm::vec3>& colors,
                       GLenum drawMode = GL_LINE_STRIP);

private:
    struct DebugShape {
        std::vector<glm::vec3> points;
        std::vector<glm::vec3> colors;
    };

    enum GBufferTexture { Position = 0, Normal, Albedo, Metallic, Roughness, COUNT };

    SDL_GLContext mContext;
    unsigned mCurrentTime = 0;
    glm::ivec2 mScreenSize = {0, 0};

    std::unordered_map<GLenum, std::vector<DebugShape>>
        mDebugShapes; // Mapped by draw mode

    GLuint mDeferredFramebuffer = 0;
    GLuint mDeferredTextures[GBufferTexture::COUNT]{};
    GLuint mDeferredDepthBuffer = 0;
    GLuint mPostProcessFramebuffer = 0;
    GLuint mPostProcessTexture = 0;
    GLuint mPostProcessDepthBuffer = 0;
    ShaderResource::CPtr mPostProcessShader;

    RenderingSys(const RenderingSys&) = delete;
    RenderingSys& operator=(const RenderingSys&) = delete;
    RenderingSys(RenderingSys&&) = delete;
    RenderingSys& operator=(RenderingSys&&) = delete;

    void initGL(SDL_Window* window);
    void initDeferredRendering();
    void initPostProcessRendering();
    void renderRenderable(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                          const PositionComp& position, const RenderableComp& renderable);
    void renderLight(const glm::mat4& viewMatrix, const PositionComp& position,
                     const LightComp& light);
    void renderPostProcessing();
    void renderDebugShape(const ShaderResource& shader, const glm::mat4& viewMatrix,
                          const glm::mat4& projectionMatrix, const DebugShape& shape,
                          GLenum drawMode);
    void drawBoundingBoxes();
    void cloneDepthBuffer(GLuint source, GLuint dest);
    glm::mat4 getModelMatrix(const PositionComp& position);
    glm::mat4 getViewMatrix(const CameraEntity& camera);
    glm::mat4 getProjectionMatrix(const CameraEntity& camera);
};
