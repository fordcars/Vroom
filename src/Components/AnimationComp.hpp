#pragma once
#include "Systems/ResourceSys/AnimationResource.hpp"

enum class AnimationMode { OneShot, Loop };

struct AnimationComp {
    AnimationResource resource; // Copy
    float currentTime;
    AnimationMode mode;
};