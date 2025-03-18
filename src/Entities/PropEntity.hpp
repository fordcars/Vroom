#pragma once

#include <vector>

#include "Components/FrictionComp.hpp"
#include "Components/PhysicsComp.hpp"
#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entity.hpp"

class PropEntity
    : public Entity<PositionComp, RenderableComp, FrictionComp, BoxPhysicsComp> {
public:
    static std::vector<PropEntity> instances;
};