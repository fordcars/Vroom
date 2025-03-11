#pragma once
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <optional>

#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entities/CameraEntity.hpp"

class RenderingSys {
public:
    static RenderingSys& get();
    RenderingSys() = default;
    ~RenderingSys();

    bool init(SDL_Window* window);
    void clear();
    void render(SDL_Window* window);

private:
    SDL_GLContext mContext;
    unsigned mCurrentTime = 0;

    RenderingSys(const RenderingSys&) = delete;
    RenderingSys& operator=(const RenderingSys&) = delete;
    RenderingSys(RenderingSys&&) = delete;
    RenderingSys& operator=(RenderingSys&&) = delete;

    void initGL(SDL_Window* window);
    void renderEntity(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                      const PositionComp& position, const RenderableComp& renderable);
    glm::mat4 getModelMatrix(const PositionComp& position);
    glm::mat4 getViewMatrix(const CameraEntity& camera);
    glm::mat4 getProjectionMatrix(const CameraEntity& camera);
};
