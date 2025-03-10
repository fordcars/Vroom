#pragma once
#include <string>

enum class AnimationMode { OneShot, Loop };

struct AnimationComp {
    std::string currentAnimation;
    std::string previousAnimation;
    AnimationMode mode = AnimationMode::Loop;

    float currentTime = 0.0;
    float startTime = -1.0f;
    float endTime = -1.0f;
    float speed = 1.0f;
    float crossfadeTime = 0.0f;
    float crossfadeDuration = 1.00f;

    void setAnimation(const std::string& animName,
                      AnimationMode mode = AnimationMode::Loop) {
        if(currentAnimation != animName) {
            currentAnimation = animName;
            currentTime = 0.0f;
        }

        this->mode = mode;
    }
};