#include "PhysicsSys.hpp"

#include <memory>

#include "Components/FrictionComp.hpp"
#include "Components/MotionComp.hpp"
#include "Components/PositionComp.hpp"
#include "Entities/EntityFilter.hpp"

// Static
PhysicsSys& PhysicsSys::get() {
    static std::unique_ptr<PhysicsSys> instance = std::make_unique<PhysicsSys>();
    return *instance;
}

void PhysicsSys::update(float deltaTime) {
    EntityFilter<PositionComp, MotionComp, std::optional<FrictionComp>> filter;
    for(const auto& [position, motion, frictionComp] : filter) {
        // Compute velocity in 2 steps to better handle large delta times
        motion.velocity += 0.5f * motion.acceleration * deltaTime;
        position.coords += motion.velocity * deltaTime;
        motion.velocity += 0.5f * motion.acceleration * deltaTime;

        if(frictionComp) {
            FrictionComp& friction = frictionComp->get();
            motion.velocity *= std::exp(-friction.coefficient * deltaTime);
        }
    }
}
