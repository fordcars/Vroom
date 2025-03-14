#pragma once

#include <SDL2/SDL.h>

class UISys {
public:
    static UISys& get();
    UISys() = default;

    bool init(SDL_Window* window, SDL_GLContext gl_context);
    void handleEvent(const SDL_Event& event);
    void update(float deltaTime);
    void render();
    void shutdown();

private:
    UISys(const UISys&) = delete;
    UISys& operator=(const UISys&) = delete;
    UISys(UISys&&) = delete;
    UISys& operator=(UISys&&) = delete;
};
