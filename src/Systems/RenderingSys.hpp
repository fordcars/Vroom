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

    void addDebugShape(const std::vector<glm::vec3>& points,
                                const std::vector<glm::vec3>& colors,
                                GLenum drawMode = GL_LINE_STRIP);
    void toggleBoundingBoxes();

private:
    struct DebugShape {
        std::vector<glm::vec3> points;
        std::vector<glm::vec3> colors;
    };

    enum GBufferTexture { Position, Normal, Albedo, Metallic, Roughness, COUNT };

    SDL_GLContext mContext;
    unsigned mCurrentTime = 0;
    glm::ivec2 mScreenSize = {0, 0};

    std::unordered_map<GLenum, std::vector<DebugShape>> mDebugShapes; // Mapped by draw mode
    bool mShowBoundingBoxes = false;

    GLuint mDeferredFramebuffer = 0;
    GLuint mDeferredTextures[GBufferTexture::COUNT]{};
    GLuint mDeferredDepthbuffer = 0;

    RenderingSys(const RenderingSys&) = delete;
    RenderingSys& operator=(const RenderingSys&) = delete;
    RenderingSys(RenderingSys&&) = delete;
    RenderingSys& operator=(RenderingSys&&) = delete;

    void initGL(SDL_Window* window);
    void initDeferredRendering();
    void renderRenderable(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                          const PositionComp& position, const RenderableComp& renderable);
    void renderLight(const glm::mat4& viewMatrix, const PositionComp& position,
                     const LightComp& light);
    void renderDebugShape(const ShaderResource& shader, const glm::mat4& viewMatrix,
                             const glm::mat4& projectionMatrix,
                             const DebugShape& shape, GLenum drawMode);
    void drawBoundingBoxes();
    void cloneDepthBuffer(GLuint source, GLuint dest);
    glm::mat4 getModelMatrix(const PositionComp& position);
    glm::mat4 getViewMatrix(const CameraEntity& camera);
    glm::mat4 getProjectionMatrix(const CameraEntity& camera);
};
