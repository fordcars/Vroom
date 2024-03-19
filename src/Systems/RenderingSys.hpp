#pragma once
#include <SDL2/SDL.h>

class RenderingSys {
public:
    static RenderingSys& get();

    RenderingSys() = default;
    ~RenderingSys();
    RenderingSys(const RenderingSys&) = delete;
    RenderingSys& operator=(const RenderingSys&) = delete;
    RenderingSys(RenderingSys&&) = delete;
    RenderingSys& operator=(RenderingSys&&) = delete;

    bool init(SDL_Window* window);
    void clear();
    void render(SDL_Window* window);

private:
    SDL_GLContext mContext;
    void initGL(SDL_Window* window);
};
