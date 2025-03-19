#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Components/PhysicsComp.hpp"

class PhysicsSys {
public:
    static PhysicsSys& get();
    PhysicsSys() = default;
    void update(float deltaTime);

    template <typename EntityT>
    void applyImpulse(EntityT& entity, const glm::vec3& impulse) {
        auto& motion = entity.template get<PhysicsComp>();
        motion.velocity += impulse;
    }

    void toggleCollisionShapes();

private:
    bool mShowCollisionShapes = false;

    PhysicsSys(const PhysicsSys&) = delete;
    PhysicsSys& operator=(const PhysicsSys&) = delete;
    PhysicsSys(PhysicsSys&&) = delete;
    PhysicsSys& operator=(PhysicsSys&&) = delete;

    void drawCollisionShapes();
};
