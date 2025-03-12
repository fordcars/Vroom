#pragma once
#include "CameraEntity.hpp"
#include "LightEntity.hpp"
#include "PlayerEntity.hpp"
#include "PropEntity.hpp"

template <typename... EntityTs>
class EntityRegistry {};

using EntityRegistryDefinition =
    EntityRegistry<CameraEntity, LightEntity, PlayerEntity, PropEntity>;
