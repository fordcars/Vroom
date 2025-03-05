#pragma once
#include <string>

enum class AnimationMode { OneShot, Loop };

struct AnimationComp {
    std::string currentAnimation;
    std::string previousAnimation;
    AnimationMode mode = AnimationMode::Loop;
    float currentTime = 0.0;
    float crossfadeTime = 0.0f;
    float crossfadeDuration = 0.25f;
};