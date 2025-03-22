#pragma once
#include <glm/glm.hpp>
#include <optional>
#include <vector>

#include "Components/FrictionComp.hpp"
#include "Components/GravityComp.hpp"
#include "Components/PhysicsComp.hpp"
#include "Components/PositionComp.hpp"

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

    void updatePhysics(
        PhysicsComp& physics,
        const std::optional<std::reference_wrapper<FrictionComp>>& frictionComp,
        const std::optional<std::reference_wrapper<GravityComp>>& gravityComp, float deltaTime);
    void handleSphereEntityCollision(
        PositionComp& position, SpherePhysicsComp& physics,
        const std::optional<std::reference_wrapper<FrictionComp>>& frictionComp,
        const std::optional<std::reference_wrapper<GravityComp>>& gravityComp,
        float deltaTime);

    void drawCollisionShapes();
};
