#pragma once

#include <vector>

#include "Components/LightComp.hpp"
#include "Components/MotionComp.hpp"
#include "Components/PositionComp.hpp"
#include "Entity.hpp"

class LightEntity : public Entity<PositionComp, LightComp, MotionComp> {
public:
    static std::vector<LightEntity> instances;
};