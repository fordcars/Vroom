#pragma once
#include <string>

enum class AnimationMode { OneShot, Loop };

struct AnimationComp {
    std::string currentAnimation;
    AnimationMode mode = AnimationMode::Loop;
    float currentTime = 0.0;
};