#include "InputSys.hpp"

#include <memory>

// Static
InputSys& InputSys::get() {
    static std::unique_ptr<InputSys> instance = std::make_unique<InputSys>();
    return *instance;
}

InputSys::InputSys() { initializeDefaultMappings(); }

void InputSys::handleEvent(const SDL_Event& event) {
    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        handleKeyboardInput(event);
    } else if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP ||
              event.type == SDL_MOUSEMOTION) {
        handleMouseInput(event);
    }
}

void InputSys::initializeDefaultMappings() {
    mInputMapping[SDLK_SPACE] = InputNeed::Jump;
    mInputMapping[SDLK_LEFT] = InputNeed::WalkLeft;
    mInputMapping[SDLK_RIGHT] = InputNeed::WalkRight;
    mInputMapping[SDLK_DOWN] = InputNeed::Crouch;
}

void InputSys::handleKeyboardInput(const SDL_Event& event) {
    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDL_Keycode key = event.key.keysym.sym;
        auto it = mInputMapping.find(key);
        if(it != mInputMapping.end()) {
            InputNeed inputNeed = it->second;
            switch(inputNeed) {
                case InputNeed::Jump:
                    // Handle Jump
                    break;
                case InputNeed::WalkLeft:
                    // Handle WalkLeft
                    break;
                case InputNeed::WalkRight:
                    // Handle WalkRight
                    break;
                case InputNeed::Crouch:
                    // Handle Crouch
                    break;
            }
        }
    }
}

void InputSys::handleMouseInput(const SDL_Event& event) {
    // Handle mouse input
}
