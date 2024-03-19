#pragma once
#include "CarEntity.hpp"
#include "CameraEntity.hpp"

template<typename... EntityTs>
class EntityRegistry {};

using EntityRegistryDefinition = EntityRegistry<
    CarEntity,
    CameraEntity
>;
