#pragma once
#include "Systems/ResourceSys/Obj/ObjAnimation.hpp"

enum class AnimationMode { OneShot, Loop };

struct AnimationComp {
    ObjAnimation resource; // Copy
    float currentTime;
    AnimationMode mode;
};