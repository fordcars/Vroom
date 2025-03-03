#pragma once

#include <vector>

#include "Components/AnimationComp.hpp"
#include "Components/FrictionComp.hpp"
#include "Components/MotionComp.hpp"
#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entity.hpp"

class PlayerEntity : public Entity<PositionComp, RenderableComp, AnimationComp,
                                   MotionComp, FrictionComp> {
public:
    static std::vector<PlayerEntity> instances;
};