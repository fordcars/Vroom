#pragma once

#include <cstddef>

enum class AnimationMode { OneShot, Loop };

struct AnimationComp {
    std::size_t currentAnimation;
    std::size_t previousAnimation;
    AnimationMode mode = AnimationMode::Loop;

    float currentTime = 0.0;
    float startTime = -1.0f;
    float endTime = -1.0f;
    float speed = 1.0f;
    float crossfadeTime = 0.0f;
    float crossfadeDuration = 1.00f;

    void setAnimation(std::size_t animNameIndex,
                      AnimationMode mode = AnimationMode::Loop) {
        if(currentAnimation != animNameIndex) {
            currentAnimation = animNameIndex;
            currentTime = 0.0f;
        }

        this->mode = mode;
    }
};