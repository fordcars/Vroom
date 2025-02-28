#pragma once

#include <vector>

#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entity.hpp"

class CarEntity : public Entity<PositionComp, RenderableComp> {
public:
    static std::vector<CarEntity> instances;
};