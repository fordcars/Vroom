#include "EventSys.hpp"

#include <SDL2/SDL.h>

#include <memory>

#include "InputSys.hpp"
#include "UISys.hpp"

// Static
EventSys& EventSys::get() {
    static std::unique_ptr<EventSys> instance = std::make_unique<EventSys>();
    return *instance;
}

// Returns false if the game should quit
bool EventSys::handleEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        // Handle quit event
        if(event.type == SDL_QUIT) {
            return false;
        }

        UISys::get().handleEvent(event);

        // Handle input events
        if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP ||
           event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP ||
           event.type == SDL_MOUSEMOTION) {
            InputSys::get().handleEvent(event);
        }
    }

    return true;
}