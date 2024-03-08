#pragma once

#include <SDL2/SDL.h>

class Game {
public:
    Game();
    void init();

private:
    SDL_Window* mMainWindow; // We might have multiple windows one day
	SDL_GLContext mMainContext; // OpenGl context
};
