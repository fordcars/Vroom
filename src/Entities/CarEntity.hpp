#pragma once

#include <vector>
#include "Entity.hpp"
#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"

class CarEntity : public Entity<PositionComp, RenderableComp> {
public:
    static std::vector<CarEntity> instances;
};