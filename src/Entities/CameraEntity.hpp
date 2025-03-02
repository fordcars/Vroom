#pragma once

#include <vector>

#include "Components/CameraInfoComp.hpp"
#include "Components/MotionComp.hpp"
#include "Components/PositionComp.hpp"
#include "Entity.hpp"

class CameraEntity : public Entity<PositionComp, MotionComp, CameraInfoComp> {
public:
    static std::vector<CameraEntity> instances;
};