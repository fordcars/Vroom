#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Components/MotionComp.hpp"

class PhysicsSys {
public:
    static PhysicsSys& get();
    PhysicsSys() = default;
    void update(float deltaTime);

    template <typename EntityT>
    void applyImpulse(EntityT& entity, const glm::vec3& impulse) {
        auto& motion = entity.template get<MotionComp>();
        motion.velocity += impulse;
    }

private:
    PhysicsSys(const PhysicsSys&) = delete;
    PhysicsSys& operator=(const PhysicsSys&) = delete;
    PhysicsSys(PhysicsSys&&) = delete;
    PhysicsSys& operator=(PhysicsSys&&) = delete;
};
