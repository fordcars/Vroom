#pragma once

#include <vector>
#include "Entity.hpp"
#include "Components/PositionComp.hpp"
#include "Components/TempComp.hpp"

class TempEntity : public Entity<TempComp, PositionComp> {
public:
    static std::vector<TempEntity> instances;
};