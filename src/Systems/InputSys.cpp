#include "InputSys.hpp"

#include <memory>

#include "Components/MotionComp.hpp"
#include "Entities/PlayerEntity.hpp"
#include "Systems/PhysicsSys.hpp"

// Static
InputSys& InputSys::get() {
    static std::unique_ptr<InputSys> instance = std::make_unique<InputSys>();
    return *instance;
}

void InputSys::init() {
    mInputMapping[SDLK_SPACE] = InputNeed::Jump;
    mInputMapping[SDLK_LEFT] = InputNeed::WalkLeft;
    mInputMapping[SDLK_RIGHT] = InputNeed::WalkRight;
    mInputMapping[SDLK_UP] = InputNeed::WalkForward;
    mInputMapping[SDLK_DOWN] = InputNeed::WalkBackward;
    mInputMapping[SDLK_LCTRL] = InputNeed::Crouch;
}

void InputSys::update() {
    for(auto key : mHeldKeys) {
        auto it = mInputMapping.find(key);
        if(it != mInputMapping.end()) {
            handleHoldNeed(it->second);
        }
    }
}

void InputSys::handleEvent(const SDL_Event& event) {
    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDL_Keycode key = event.key.keysym.sym;
        auto it = mInputMapping.find(key);
        if(it != mInputMapping.end()) {
            if(event.type == SDL_KEYDOWN) {
                if(mHeldKeys.find(key) == mHeldKeys.end()) {
                    handleDownNeed(it->second);
                    mHeldKeys.insert(key);
                }
            } else {
                handleUpNeed(it->second);
                mHeldKeys.erase(key);
            }
        }
    } else if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP ||
              event.type == SDL_MOUSEMOTION) {
        handleMouseInput(event);
    }
}

// Handle key down event
void InputSys::handleDownNeed(InputNeed need) {
    switch(need) {
        case InputNeed::Jump:
            PhysicsSys::get().applyImpulse(PlayerEntity::instances[0], {0, 2, 0});
            break;
    }
}

// Handle key up event
void InputSys::handleUpNeed(InputNeed need) {
    switch(need) {
        case InputNeed::WalkLeft:
        case InputNeed::WalkRight:
            PlayerEntity::instances[0].get<MotionComp>().velocity.x = 0;
            break;
        case InputNeed::WalkForward:
        case InputNeed::WalkBackward:
            PlayerEntity::instances[0].get<MotionComp>().velocity.z = 0;
            break;
    }
}

// Handle key hold event
void InputSys::handleHoldNeed(InputNeed need) {
    const float speed = 2;
    switch(need) {
        case InputNeed::WalkLeft:
            PlayerEntity::instances[0].get<MotionComp>().velocity.x = -speed;
            break;
        case InputNeed::WalkRight:
            PlayerEntity::instances[0].get<MotionComp>().velocity.x = speed;
            break;
        case InputNeed::WalkForward:
            PlayerEntity::instances[0].get<MotionComp>().velocity.z = speed;
            break;
        case InputNeed::WalkBackward:
            PlayerEntity::instances[0].get<MotionComp>().velocity.z = -speed;
            break;
        case InputNeed::Crouch:
            PlayerEntity::instances[0].get<PositionComp>().coords.y -= speed / 60;
            break;
    }
}

void InputSys::handleMouseInput(const SDL_Event& event) {
    // Handle mouse input
}
