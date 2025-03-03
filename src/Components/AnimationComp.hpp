#pragma once
#include "Systems/ResourceSys/Obj/ObjAnimation.hpp"

enum class AnimationMode { OneShot, Loop };

struct AnimationComp {
    ObjAnimation::Ptr objAnimation;
    ObjAnimation::Animation* currentAnimation = nullptr;
    AnimationMode mode = AnimationMode::Loop;
    float currentTime = 0.0;
};