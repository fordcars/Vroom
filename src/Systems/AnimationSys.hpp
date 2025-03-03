#pragma once

#include "Components/AnimationComp.hpp"
#include "Entities/EntityFilter.hpp"

class AnimationSys {
public:
    static AnimationSys& get();
    AnimationSys() = default;
    void update(float deltaTime);

private:
    void updateAnimation(AnimationComp& animationComp, float deltaTime);
};
