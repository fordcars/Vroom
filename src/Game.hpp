#pragma once

#include <SDL2/SDL.h>

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
    SDL_Window* mMainWindow;
    bool mQuitting = false;
    Uint32 mLastFrameTime;

    bool init();
    void quit();
    void doMainLoop();
    void checkForErrors();
};
