#include "InputSys.hpp"

#include <cmath>
#include <glm/gtx/norm.hpp>
#include <memory>

#include "Components/AnimationComp.hpp"
#include "Components/MotionComp.hpp"
#include "Entities/LightEntity.hpp"
#include "Entities/PlayerEntity.hpp"
#include "Systems/PhysicsSys.hpp"
#include "Systems/RenderingSys.hpp"
#include "Systems/ResourceSys/ResourceSys.hpp"
#include "Systems/UISys.hpp"
#include "Utils/MathUtils.hpp"

// Static
InputSys& InputSys::get() {
    static std::unique_ptr<InputSys> instance = std::make_unique<InputSys>();
    return *instance;
}

bool InputSys::init() {
    mInputMapping[SDLK_SPACE] = InputNeed::Jump;
    mInputMapping[SDLK_LEFT] = InputNeed::WalkLeft;
    mInputMapping[SDLK_RIGHT] = InputNeed::WalkRight;
    mInputMapping[SDLK_UP] = InputNeed::WalkForward;
    mInputMapping[SDLK_DOWN] = InputNeed::WalkBackward;
    mInputMapping[SDLK_LSHIFT] = InputNeed::Run;
    mInputMapping[SDLK_LCTRL] = InputNeed::Crouch;

    // Debug stuff
    mInputMapping[SDLK_F1] = InputNeed::ToggleShowFPS;
    mInputMapping[SDLK_F2] = InputNeed::ChangeDebugRenderMode;
    mInputMapping[SDLK_F3] = InputNeed::ToggleShowWalkVectors;
    return true;
}

void InputSys::update(float deltaTime) {
    mWalkDirection = {};
    mRunning = false;

    for(auto key : mHeldKeys) {
        auto it = mInputMapping.find(key);
        if(it != mInputMapping.end()) {
            handleHoldNeed(it->second);
        }
    }

    handleWalking(deltaTime);
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
        case InputNeed::ToggleShowFPS:
            UISys::get().toggleFPSOverlay();
            break;
        case InputNeed::ChangeDebugRenderMode:
            cycleDebugRenderMode();
            break;
        case InputNeed::ToggleShowWalkVectors:
            mShowDebugWalkVectors = !mShowDebugWalkVectors;
            if(mShowDebugWalkVectors) {
                Log::debug() << "Showing walk vectors.";
            } else {
                Log::debug() << "Hiding walk vectors.";
            }
            break;
        default:
            break;
    }
}

// Handle key hold event
void InputSys::handleHoldNeed(InputNeed need) {
    const float speed = 2;
    switch(need) {
        case InputNeed::WalkLeft:
            mWalkDirection.x = -1;
            break;
        case InputNeed::WalkRight:
            mWalkDirection.x = 1;
            break;
        case InputNeed::WalkForward:
            mWalkDirection.z = -1;
            break;
        case InputNeed::WalkBackward:
            mWalkDirection.z = 1;
            break;
        case InputNeed::Run:
            mRunning = true;
            break;
        case InputNeed::Crouch:
            PlayerEntity::instances[0].get<PositionComp>().coords.y -= speed / 60;
            break;
    }
}

void InputSys::handleMouseInput(const SDL_Event& event) {
    // Handle mouse input
}

void InputSys::handleWalking(float deltaTime) {
    const float TARGET_WALK_SPEED = 6.0f;
    const float TARGET_RUN_SPEED = 15.0f;
    const float WALK_ACCELERATION = 50.0f;
    const float RUN_ACCELERATION = 130.0f;
    const float ANTI_JARRING = 3.0f;

    float targetSpeed = mRunning ? TARGET_RUN_SPEED : TARGET_WALK_SPEED;
    float targetSpeedSq = targetSpeed * targetSpeed;
    float acceleration = mRunning ? RUN_ACCELERATION : WALK_ACCELERATION;

    auto& motionComp = PlayerEntity::instances[0].get<MotionComp>();
    auto& animationComp = PlayerEntity::instances[0].get<AnimationComp>();
    auto& positionComp = PlayerEntity::instances[0].get<PositionComp>();

    float currentSpeedSq = glm::length2(motionComp.velocity);

    if(mWalkDirection.x != 0 || mWalkDirection.z != 0) {
        if(Utils::floatsEqualish(motionComp.velocity.x, 0, 0.3) &&
           Utils::floatsEqualish(motionComp.velocity.z, 0, 0.3)) {
            // If current motion is around 0, add a small amount of velocity in the
            // current direction to prevent a jarring rotation transition. Also is more
            // realistic.
            const float& angle = positionComp.rotation.y;
            motionComp.velocity +=
                glm::vec3{std::sin(angle), 0, std::cos(angle)} * ANTI_JARRING;
        }
        mWalkDirection = glm::normalize(mWalkDirection);

        // Apply walk acceleration if we are under target speed or currently turning
        float motionAndDirectionSimilarity =
            glm::dot(glm::normalize(motionComp.velocity), mWalkDirection);
        if(currentSpeedSq < targetSpeedSq) {
            motionComp.velocity += (mWalkDirection * acceleration) * deltaTime;
        } else if(!Utils::floatsEqualish(motionAndDirectionSimilarity, 1, 0.01)) {
            // Already at max speed, but still turning. Apply rotation, but keep speed
            // constant.
            float currentSpeed = std::sqrt(currentSpeedSq);
            motionComp.velocity += (mWalkDirection * acceleration) * deltaTime;
            motionComp.velocity = glm::normalize(motionComp.velocity) * currentSpeed;
        }

        // Set rotation based on velocity direction
        if(motionComp.velocity.x != 0 || motionComp.velocity.z != 0) {
            positionComp.rotation.y =
                std::atan2(motionComp.velocity.x, motionComp.velocity.z);
        }

        if(mShowDebugWalkVectors) {
            RenderingSys::get().addDebugShape(
                {positionComp.coords, mWalkDirection * 3.0f + positionComp.coords},
                std::vector<glm::vec3>(2, {1, 0, 0}));
        }
    }

    if(mShowDebugWalkVectors) {
        RenderingSys::get().addDebugShape(
            {positionComp.coords, motionComp.velocity + positionComp.coords},
            std::vector<glm::vec3>(2, {0, 1, 0}));
        RenderingSys::get().addDebugShape(
            Utils::generateCircle(2, positionComp.coords, {0, 1, 0}, 32),
            std::vector<glm::vec3>(32, {0, 1, 0}), GL_LINE_LOOP);
    }

    if(currentSpeedSq < 0.4) {
        animationComp.setAnimation(Constants::AnimationName::get<"Happy">());
        animationComp.speed = 1.0f;
    } else {
        animationComp.setAnimation(Constants::AnimationName::get<"Normal Walk">());
        animationComp.speed = currentSpeedSq / (TARGET_WALK_SPEED * TARGET_WALK_SPEED);
    }
}

void InputSys::cycleDebugRenderMode() {
    switch(mDebugRenderMode) {
        case 0:
            ++mDebugRenderMode;
            LightEntity::instances[0].get<LightComp>().shader =
                ResourceSys::get().getShaderResource("test_lightPosition");
            Log::debug() << "Debug render mode: worldspace position";
            break;
        case 1:
            ++mDebugRenderMode;
            LightEntity::instances[0].get<LightComp>().shader =
                ResourceSys::get().getShaderResource("test_lightNormal");
            Log::debug() << "Debug render mode: cameraspace normal";
            break;
        case 2:
            ++mDebugRenderMode;
            LightEntity::instances[0].get<LightComp>().shader =
                ResourceSys::get().getShaderResource("test_lightAlbedo");
            Log::debug() << "Debug render mode: albedo";
            break;
        case 3:
            ++mDebugRenderMode;
            LightEntity::instances[0].get<LightComp>().shader =
                ResourceSys::get().getShaderResource("test_lightMetallic");
            Log::debug() << "Debug render mode: metallic";
            break;
        case 4:
            ++mDebugRenderMode;
            LightEntity::instances[0].get<LightComp>().shader =
                ResourceSys::get().getShaderResource("test_lightRoughness");
            Log::debug() << "Debug render mode: roughness";
            break;
        default:
            mDebugRenderMode = 0;
            LightEntity::instances[0].get<LightComp>().shader =
                ResourceSys::get().getShaderResource("light_pbr");
            Log::debug() << "Debug render mode disabled.";
            break;
    }
}
