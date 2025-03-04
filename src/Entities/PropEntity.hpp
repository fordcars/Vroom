#pragma once

#include <vector>

#include "Components/FrictionComp.hpp"
#include "Components/MotionComp.hpp"
#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entity.hpp"

class PropEntity : public Entity<PositionComp, RenderableComp, MotionComp, FrictionComp> {
public:
    static std::vector<PropEntity> instances;
};