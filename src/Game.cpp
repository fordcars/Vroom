#include "Game.hpp"
#include <SDL2/SDL.h>

#include "Log.hpp"

Game::Game() {}

bool Game::init() {
    Log::info() << "Initializing game...";

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
	{
		Log::sdl_error() << "Unable to initialize SDL";
        return false;
	}
    return true;
}
