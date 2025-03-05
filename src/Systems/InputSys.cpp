#include "InputSys.hpp"

#include <cmath>
#include <glm/gtx/norm.hpp>
#include <memory>

#include "Components/AnimationComp.hpp"
#include "Components/MotionComp.hpp"
#include "Entities/PlayerEntity.hpp"
#include "Systems/PhysicsSys.hpp"
#include "Utils/MathUtils.hpp"

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
    mUpdateWalkDirection = {};

    for(auto key : mHeldKeys) {
        auto it = mInputMapping.find(key);
        if(it != mInputMapping.end()) {
            handleHoldNeed(it->second);
        }
    }

    handleWalking();
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
        default:
            break;
    }
}

// Handle key hold event
void InputSys::handleHoldNeed(InputNeed need) {
    const float speed = 2;
    switch(need) {
        case InputNeed::WalkLeft:
            mUpdateWalkDirection.x = -1;
            break;
        case InputNeed::WalkRight:
            mUpdateWalkDirection.x = 1;
            break;
        case InputNeed::WalkForward:
            mUpdateWalkDirection.z = -1;
            break;
        case InputNeed::WalkBackward:
            mUpdateWalkDirection.z = 1;
            break;
        case InputNeed::Crouch:
            PlayerEntity::instances[0].get<PositionComp>().coords.y -= speed / 60;
            break;
    }
}

void InputSys::handleMouseInput(const SDL_Event& event) {
    // Handle mouse input
}

void InputSys::handleWalking() {
    const float MAX_SPEED = 4;
    const float MAX_SPEED_SQ = MAX_SPEED * MAX_SPEED;

    const float ACCELERATION = 0.4;
    auto& motionComp = PlayerEntity::instances[0].get<MotionComp>();
    auto& animationComp = PlayerEntity::instances[0].get<AnimationComp>();
    auto& positionComp = PlayerEntity::instances[0].get<PositionComp>();

    float currentSpeedSq = glm::length2(motionComp.velocity);

    if(mUpdateWalkDirection.x != 0 || mUpdateWalkDirection.z != 0) {
        if(Utils::floatsEqualish(motionComp.velocity.x, 0, 0.3) &&
           Utils::floatsEqualish(motionComp.velocity.z, 0, 0.3)) {
            // Add a small amount of velocity in the current direction to
            // prevent a jarring rotation transition. Also is more realistic.
            Log::debug() << "Adding initial velocity";
            float angle = positionComp.rotation.y * M_PI / 180;
            motionComp.velocity += glm::vec3{std::sin(angle), 0, std::cos(angle)} * 1.0f;
        }
        mUpdateWalkDirection = glm::normalize(mUpdateWalkDirection);

        // Apply walk acceleration
        if(currentSpeedSq < MAX_SPEED_SQ) {
            motionComp.velocity += mUpdateWalkDirection * ACCELERATION;
        }

        // Set rotation based on velocity direction
        if(motionComp.velocity.x != 0 || motionComp.velocity.z != 0) {
            positionComp.rotation.y =
                std::atan2(motionComp.velocity.x, motionComp.velocity.z) * 180 / M_PI;
        }
    }

    if(currentSpeedSq < 0.01) {
        animationComp.currentAnimation = "Happy";
    } else {
        animationComp.currentAnimation = "Normal Walk";
    }
}
