#pragma once

#include <vector>
#include "Entity.hpp"
#include "Components/PositionComp.hpp"
#include "Components/CameraInfoComp.hpp"

class CameraEntity : public Entity<PositionComp, CameraInfoComp> {
public:
    static std::vector<CameraEntity> instances;
};