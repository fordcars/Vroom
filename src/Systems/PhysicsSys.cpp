#include "PhysicsSys.hpp"

#include <memory>

#include "Components/FrictionComp.hpp"
#include "Components/PhysicsComp.hpp"
#include "Components/PositionComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entities/EntityFilter.hpp"

// Static
PhysicsSys& PhysicsSys::get() {
    static std::unique_ptr<PhysicsSys> instance = std::make_unique<PhysicsSys>();
    return *instance;
}

void PhysicsSys::update(float deltaTime) {
    {
        // Entities with no collisions
        EntityFilter<PositionComp, NullPhysicsComp, std::optional<FrictionComp>> filter;
        for(const auto& [position, physics, frictionComp] : filter) {
            // Compute velocity in 2 steps to better handle large delta times
            physics.velocity += 0.5f * physics.acceleration * deltaTime;
            position.coords += physics.velocity * deltaTime;
            physics.velocity += 0.5f * physics.acceleration * deltaTime;

            if(frictionComp) {
                FrictionComp& friction = frictionComp->get();
                physics.velocity *= std::exp(-friction.coefficient * deltaTime);
            }
        }
    }
    {
        // Point entities
        EntityFilter<PositionComp, PointPhysicsComp, std::optional<FrictionComp>> filter;
        for(const auto& [position, physics, frictionComp] : filter) {
        }
    }

    {
        // Sphere entities
        EntityFilter<PositionComp, SpherePhysicsComp, std::optional<FrictionComp>> filter;
        for(const auto& [position, physics, frictionComp] : filter) {
        }
    }
    {
        // Box entities
        EntityFilter<PositionComp, BoxPhysicsComp, std::optional<FrictionComp>> filter;
        for(const auto& [position, physics, frictionComp] : filter) {
        }
    }
}
