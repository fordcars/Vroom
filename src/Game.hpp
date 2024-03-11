#pragma once

#include <SDL2/SDL.h>
#include "Systems/RenderingSys.hpp"
#include "Systems/ResourceSys/ResourceSys.hpp"

class Game {
public:
    Game();
    ~Game();
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;
    bool start();

private:
    RenderingSys mRenderingSys;
    ResourceSys mResourceSys;

    SDL_Window* mMainWindow;
    bool mQuitting = false;

    bool init();
    void quit();
    void doMainLoop();
    void doEvents();
    void checkForErrors();
};
