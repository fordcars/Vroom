#pragma once
#include <SDL2/SDL.h>

class Renderer {
public:
    Renderer() = default;
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    bool init(SDL_Window* window);
    void clear();
    void render(SDL_Window* window);

private:
    SDL_GLContext mContext;
};
