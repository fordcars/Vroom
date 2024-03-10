#pragma once
#include <SDL2/SDL.h>

class RenderingSystem {
public:
    RenderingSystem() = default;
    ~RenderingSystem();
    RenderingSystem(const RenderingSystem&) = delete;
    RenderingSystem& operator=(const RenderingSystem&) = delete;
    RenderingSystem(RenderingSystem&&) = delete;
    RenderingSystem& operator=(RenderingSystem&&) = delete;

    bool init(SDL_Window* window);
    void clear();
    void render(SDL_Window* window);

private:
    SDL_GLContext mContext;
    void initGL(SDL_Window* window);
};
