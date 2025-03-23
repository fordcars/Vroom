#include "InputSys.hpp"

#include <cmath>
#include <glm/gtx/norm.hpp>
#include <memory>

#include "Components/AnimationComp.hpp"
#include "Components/PhysicsComp.hpp"
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
    mInputMapping[SDLK_F4] = InputNeed::ToggleShowCollisionShapes;
    return true;
}

void InputSys::update(float deltaTime) {
    mWalkInputDirection = {};
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
            // Jump if we are on the ground
            if(PlayerEntity::instances[0].get<PhysicsComp>().currentCollision.yNeg) {
                PhysicsSys::get().applyImpulse(PlayerEntity::instances[0], {0, 7, 0});
            }
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
        case InputNeed::ToggleShowCollisionShapes:
            PhysicsSys::get().toggleCollisionShapes();
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
            mWalkInputDirection.x = -1;
            break;
        case InputNeed::WalkRight:
            mWalkInputDirection.x = 1;
            break;
        case InputNeed::WalkForward:
            mWalkInputDirection.z = -1;
            break;
        case InputNeed::WalkBackward:
            mWalkInputDirection.z = 1;
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

    float targetSpeed = mRunning ? TARGET_RUN_SPEED : TARGET_WALK_SPEED;
    float targetSpeedSq = targetSpeed * targetSpeed;
    float acceleration = mRunning ? RUN_ACCELERATION : WALK_ACCELERATION;

    auto& physicsComp = PlayerEntity::instances[0].get<PhysicsComp>();
    auto& animationComp = PlayerEntity::instances[0].get<AnimationComp>();
    auto& positionComp = PlayerEntity::instances[0].get<PositionComp>();
    auto& soundComp = PlayerEntity::instances[0].get<SoundComp>();

    float currentSpeedSq =
        glm::length2(glm::vec2(physicsComp.velocity.x, physicsComp.velocity.z));

    if(mWalkInputDirection.x != 0 || mWalkInputDirection.z != 0) {
        mWalkInputDirection = glm::normalize(mWalkInputDirection);

        // Apply walk acceleration if we are under target speed or currently turning
        float motionAndDirectionSimilarity =
            glm::dot(glm::normalize(physicsComp.velocity), mWalkInputDirection);
        if(currentSpeedSq < targetSpeedSq) {
            physicsComp.velocity += (mWalkInputDirection * acceleration) * deltaTime;
        } else if(!Utils::floatsEqualish(motionAndDirectionSimilarity, 1, 0.01)) {
            // Already at max speed, but still turning. Apply rotation, but keep speed
            // constant.
            float currentSpeed = std::sqrt(currentSpeedSq);
            glm::vec3 walkVelocity =
                glm::vec3(physicsComp.velocity.x, 0.0f, physicsComp.velocity.z) +
                (mWalkInputDirection * acceleration) * deltaTime;
            walkVelocity = glm::normalize(walkVelocity) * currentSpeed;
            physicsComp.velocity.x = walkVelocity.x;
            physicsComp.velocity.z = walkVelocity.z;
        }

        // Set rotation based on direction
        if(physicsComp.velocity.x != 0 || physicsComp.velocity.z != 0) {
            float targetRotation =
                std::atan2(mWalkInputDirection.x, mWalkInputDirection.z);

            // Ensure interpolation takes the shortest path
            float deltaRotation =
                glm::mod(targetRotation - positionComp.rotation.y + glm::pi<float>(),
                         glm::two_pi<float>()) -
                glm::pi<float>();

            // Interpolate smoothly towards the target rotation
            positionComp.rotation.y += deltaRotation * (deltaTime * 10.0f);
        }

        if(mShowDebugWalkVectors) {
            RenderingSys::get().addDebugShape(
                {positionComp.coords, mWalkInputDirection * 3.0f + positionComp.coords},
                std::vector<glm::vec3>(2, {1, 0, 0}));
        }
    }

    if(mShowDebugWalkVectors) {
        RenderingSys::get().addDebugShape(
            {positionComp.coords, physicsComp.velocity + positionComp.coords},
            std::vector<glm::vec3>(2, {0, 1, 0}));
        RenderingSys::get().addDebugShape(
            Utils::generateCircle(2, positionComp.coords, {0, 1, 0}, 32),
            std::vector<glm::vec3>(32, {0, 1, 0}), GL_LINE_LOOP);
    }

    if(currentSpeedSq < 0.4 || !physicsComp.currentCollision.yNeg) {
        animationComp.setAnimation(Constants::AnimationName::get<"Happy">());
        animationComp.speed = 1.0f;

        if(soundComp.audioResource->call<ma_sound_is_playing>()) {
            soundComp.audioResource->call<ma_sound_stop>();
            soundComp.audioResource->call<ma_sound_seek_to_pcm_frame>(0);
        }
    } else {
        animationComp.setAnimation(Constants::AnimationName::get<"Normal Walk">());
        animationComp.speed = currentSpeedSq / (TARGET_WALK_SPEED * TARGET_WALK_SPEED);
        soundComp.audioResource->call<ma_sound_start>();
        soundComp.audioResource->call<ma_sound_set_pitch>(glm::max(animationComp.speed, 0.8f));
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
