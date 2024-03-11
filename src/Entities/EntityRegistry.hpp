#pragma once
#include "CarEntity.hpp"

template<typename... EntityTs>
class EntityRegistry {};

using EntityRegistryDefinition = EntityRegistry<
    CarEntity
>;
