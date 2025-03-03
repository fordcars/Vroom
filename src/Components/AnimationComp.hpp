#pragma once
#include "Systems/ResourceSys/Obj/ObjAnimation.hpp"

enum class AnimationMode { OneShot, Loop };

struct AnimationComp {
    ObjAnimation objAnimation; // Copy
    ObjAnimation::Animation* currentAnimation = nullptr;
    float currentTime = 0.0;
    AnimationMode mode = AnimationMode::Loop;
};