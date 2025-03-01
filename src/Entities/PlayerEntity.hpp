#pragma once

#include <vector>

#include "Components/MotionComp.hpp"
#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entity.hpp"

class PlayerEntity : public Entity<PositionComp, RenderableComp, MotionComp> {
public:
    static std::vector<PlayerEntity> instances;
};