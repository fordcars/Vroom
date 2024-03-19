#pragma once

#include <vector>
#include "Entity.hpp"
#include "Components/PositionComp.hpp"
#include "Components/CameraLensComp.hpp"

class CameraEntity : public Entity<PositionComp, CameraLensComp> {
public:
    static std::vector<CameraEntity> instances;
};