#pragma once

#include <vector>

#include "Components/CameraInfoComp.hpp"
#include "Components/PhysicsComp.hpp"
#include "Components/PositionComp.hpp"
#include "Entity.hpp"

class CameraEntity : public Entity<PositionComp, PhysicsComp, CameraInfoComp> {
public:
    static std::vector<CameraEntity> instances;
};