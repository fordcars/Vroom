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

void InputSys::handleEvent(const SDL_Event& event) {
    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDL_Keycode key = event.key.keysym.sym;
        auto it = mInputMapping.find(key);
        if(it != mInputMapping.end()) {
            handleNeed(it->second, event.type == SDL_KEYDOWN);
        }
    } else if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP ||
              event.type == SDL_MOUSEMOTION) {
        handleMouseInput(event);
    }
}

void InputSys::init() {
    mInputMapping[SDLK_SPACE] = InputNeed::Jump;
    mInputMapping[SDLK_LEFT] = InputNeed::WalkLeft;
    mInputMapping[SDLK_RIGHT] = InputNeed::WalkRight;
    mInputMapping[SDLK_UP] = InputNeed::WalkForward;
    mInputMapping[SDLK_DOWN] = InputNeed::WalkBackward;
}

void InputSys::handleNeed(InputNeed need, bool isKeyDown) {
    const float speed = 2;
    auto& physicsSys = PhysicsSys::get();

    switch(need) {
        case InputNeed::Jump:
            physicsSys.applyImpulse(PlayerEntity::instances[0], {0, 2, 0});
            break;
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
            break;
    }
}

void InputSys::handleMouseInput(const SDL_Event& event) {
    // Handle mouse input
}
