#pragma once

#include <vector>
#include "Entity.hpp"
#include "Components/PositionComp.hpp"

class CarEntity : public Entity<PositionComp> {
public:
    static std::vector<CarEntity> instances;
};