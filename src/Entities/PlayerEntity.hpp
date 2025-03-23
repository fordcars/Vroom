#pragma once

#include <vector>

#include "Components/AnimationComp.hpp"
#include "Components/FrictionComp.hpp"
#include "Components/GravityComp.hpp"
#include "Components/PhysicsComp.hpp"
#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Components/SoundComp.hpp"
#include "Entity.hpp"

class PlayerEntity : public Entity<PositionComp, RenderableComp, SoundComp, AnimationComp,
                                   SpherePhysicsComp, FrictionComp, GravityComp> {
public:
    static std::vector<PlayerEntity> instances;
};