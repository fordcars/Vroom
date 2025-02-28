#pragma once
#include "CameraEntity.hpp"
#include "CarEntity.hpp"

template <typename... EntityTs>
class EntityRegistry {};

using EntityRegistryDefinition = EntityRegistry<CarEntity, CameraEntity>;
