#pragma once

#include <vector>
#include "Entity.hpp"
#include "Components/PositionComp.hpp"
#include "Components/TempComp.hpp"

class CarEntity : public Entity<TempComp, PositionComp> {
public:
    static std::vector<CarEntity> instances;
};