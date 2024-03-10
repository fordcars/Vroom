#pragma once
#include "CarEntity.hpp"
#include "TempEntity.hpp"

template<typename... EntityTs>
class EntityRegistry {};

using EntityRegistryDefinition = EntityRegistry<
    CarEntity,
    TempEntity
>;
