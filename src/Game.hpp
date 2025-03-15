#pragma once

#include <array>
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

    static constexpr std::size_t DELTA_TIME_FRAMES_TO_AVERAGE = 5;
    Uint32 mLastFrameTime = 0;
    std::array<float, DELTA_TIME_FRAMES_TO_AVERAGE> mLastDeltaTimes = {};
    std::size_t mLastDeltaTimeIndex = 0;

    bool init();
    void requestQuit();
    void doMainLoop();
    float getCurrentDeltaTime(Uint32 startFrameTime);
    void checkForErrors();
    void shutdown();
};
