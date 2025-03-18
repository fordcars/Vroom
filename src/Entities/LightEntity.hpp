#pragma once

#include <vector>

#include "Components/LightComp.hpp"
#include "Components/PhysicsComp.hpp"
#include "Components/PositionComp.hpp"
#include "Entity.hpp"

class LightEntity : public Entity<PositionComp, LightComp, NullPhysicsComp> {
public:
    static std::vector<LightEntity> instances;
};