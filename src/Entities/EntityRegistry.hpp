#pragma once
#include "CameraEntity.hpp"
#include "PlayerEntity.hpp"

template <typename... EntityTs>
class EntityRegistry {};

using EntityRegistryDefinition = EntityRegistry<PlayerEntity, CameraEntity>;
