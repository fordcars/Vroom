#include "Game.hpp"

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "Constants.hpp"
#include "Log.hpp"
#include "Systems/AnimationSys.hpp"
#include "Systems/EventSys.hpp"
#include "Systems/GameplaySys.hpp"
#include "Systems/InputSys.hpp"
#include "Systems/PhysicsSys.hpp"
#include "Systems/RenderingSys.hpp"
#include "Systems/ResourceSys/ResourceSys.hpp"
#include "Systems/UISys.hpp"

Game::Game() {}

Game::~Game() {
    SDL_DestroyWindow(mMainWindow);
    SDL_Quit();
}

bool Game::start() {
    if(init()) {
        GameplaySys::get().start();

        mLastFrameTime = SDL_GetTicks(); // Reset this here to avoid a huge delta time on
                                         // the first frame.
        while(!mQuitting) doMainLoop();
        return true;
    }

    return false;
}

bool Game::init() {
    Log::info() << "Initializing game...";

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        Log::sdlError() << "Unable to initialize SDL!";
        return false;
    }

    // Set AA before window creation
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, Constants::AA_SAMPLES);

    // Create window
    mMainWindow = SDL_CreateWindow(
        Constants::GAME_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Constants::DEFAULT_WINDOW_SIZE_X, Constants::DEFAULT_WINDOW_SIZE_Y,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if(!mMainWindow) {
        Log::sdlError() << "Unable to create window!";
        return false;
    }

    return InputSys::get().init() && RenderingSys::get().init(mMainWindow) &&
           ResourceSys::get().loadResources();
}

void Game::requestQuit() {
    Log::info() << "Quitting!";
    mQuitting = true;
}

void Game::doMainLoop() {
    Uint32 startFrameTime = SDL_GetTicks();
    float deltaTime = getCurrentDeltaTime(startFrameTime);

    // Handle events
    if(!EventSys::get().handleEvents()) requestQuit();

    // Logic and stuff
    AnimationSys::get().update(deltaTime);
    InputSys::get().update(deltaTime);
    PhysicsSys::get().update(deltaTime);
    GameplaySys::get().update(deltaTime);
    UISys::get().update(deltaTime);

    // Rendering
    RenderingSys::get().clear();
    RenderingSys::get().render(mMainWindow);

    // Limit FPS if VSync is off
    if(!Constants::ENABLE_VSYNC) {
        Uint32 endFrameTime = SDL_GetTicks() - startFrameTime;
        if(endFrameTime < (1000 / Constants::NO_VSYNC_MAX_FPS)) {
            SDL_Delay((1000 / Constants::NO_VSYNC_MAX_FPS) - endFrameTime);
        }
    }
}

// Returns current frame's delta time, averaged over a couple of frames
// for smoothness.
float Game::getCurrentDeltaTime(Uint32 startFrameTime) {
    float rawDeltaTime = (startFrameTime - mLastFrameTime) / 1000.0f;
    mLastFrameTime = startFrameTime;

    mLastDeltaTimes[mLastDeltaTimeIndex] = rawDeltaTime;
    mLastDeltaTimeIndex = (mLastDeltaTimeIndex + 1) % 5;

    float sum = 0.0f;
    for(int i = 0; i < mLastDeltaTimes.size(); i++) {
        sum += mLastDeltaTimes[i];
    }
    return sum / 5.0f;
}

void Game::checkForErrors() {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        if(err == GL_INVALID_ENUM)
            Log::error() << "OpenGL error: GL_INVALID_ENUM";
        else if(err == GL_INVALID_VALUE)
            Log::error() << "OpenGL error: GL_INVALID_VALUE";
        else if(err == GL_INVALID_OPERATION)
            Log::error() << "OpenGL error: GL_INVALID_OPERATION";
        else if(err == GL_STACK_OVERFLOW)
            Log::error() << "OpenGL error: GL_STACK_OVERFLOW";
        else if(err == GL_STACK_UNDERFLOW)
            Log::error() << "OpenGL error: GL_STACK_UNDERFLOW";
        else if(err == GL_OUT_OF_MEMORY)
            Log::error() << "OpenGL error: GL_OUT_OF_MEMORY";
    }

    // Often, SDL errors will not be important, since it includes internal diagnostics
    std::string message = SDL_GetError();
    if(!message.empty()) {
        Log::warn() << "SDL message: " << message;
        SDL_ClearError();
    }
}

void Game::shutdown() {
    Log::info() << "Shutting down game...";
    UISys::get().shutdown();
}
