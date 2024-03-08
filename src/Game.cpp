#include "Game.hpp"
#include <SDL2/SDL.h>

#include "Log.hpp"
#include "Constants.hpp"

Game::Game() {}

bool Game::init() {
    Log::info() << "Initializing game...";

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
	{
		Log::sdl_error() << "Unable to initialize SDL";
        return false;
	}

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, Constants::OPENGL_MAJOR_VERSION);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, Constants::OPENGL_MINOR_VERSION);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	mMainWindow = SDL_CreateWindow(
        Constants::GAME_NAME,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        Constants::SIZE_X,
        Constants::SIZE_Y,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
	
	if(!mMainWindow)
	{
		Log::sdl_error() << "Unable to create window";
        return false;
	}

    return true;
}
