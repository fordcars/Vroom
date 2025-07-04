#include "PhysicsSys.hpp"

#include <memory>

#include "Components/RenderableComp.hpp"
#include "Entities/EntityFilter.hpp"
#include "RenderingSys.hpp"
#include "Utils/MathUtils.hpp"

// Static
PhysicsSys& PhysicsSys::get() {
    static std::unique_ptr<PhysicsSys> instance = std::make_unique<PhysicsSys>();
    return *instance;
}

void PhysicsSys::update(float deltaTime) {
    if(mShowCollisionShapes) {
        drawCollisionShapes();
    }

    // Reset entities
    for(auto& [physics] : EntityFilter<PhysicsComp>()) {
        physics.currentCollision = {};
    }

    // Entities with no collisions
    for(const auto& [position, physics, frictionComp, gravityComp] :
        EntityFilter<PositionComp, NullPhysicsComp, std::optional<FrictionComp>,
                     std::optional<GravityComp>>()) {
        updatePhysics(physics, frictionComp, gravityComp, deltaTime);
        position.coords += physics.velocity * deltaTime;
    }

    // Box entities (for now, behaves the same as entities with no collisions)
    for(const auto& [position, physics, renderable, frictionComp, gravityComp] :
        EntityFilter<PositionComp, BoxPhysicsComp, RenderableComp,
                     std::optional<FrictionComp>, std::optional<GravityComp>>()) {
        // Update bounding box
        auto [otherMin, otherMax] =
            renderable.objectResource->boundingBox->getWorldspaceAABB(
                glm::translate(position.getTransform(), physics.positionOffset));
        physics.minCornerVelocity = (otherMin - physics.minCorner) / deltaTime;
        physics.maxCornerVelocity = (otherMax - physics.maxCorner) / deltaTime;
        physics.minCorner = otherMin;
        physics.maxCorner = otherMax;

        updatePhysics(physics, frictionComp, gravityComp, deltaTime);
        position.coords += physics.velocity * deltaTime;
    }

    // Sphere entities
    for(auto& [position, physics, frictionComp, gravityComp] :
        EntityFilter<PositionComp, SpherePhysicsComp, std::optional<FrictionComp>,
                     std::optional<GravityComp>>()) {
        handleSphereEntityCollision(position, physics, frictionComp, gravityComp,
                                    deltaTime);
    }
}

void PhysicsSys::toggleCollisionShapes() {
    mShowCollisionShapes = !mShowCollisionShapes;
    if(mShowCollisionShapes) {
        Log::debug() << "Showing collision shapes.";
    } else {
        Log::debug() << "Hiding collision shapes.";
    }
}

void PhysicsSys::updatePhysics(
    PhysicsComp& physics,
    const std::optional<std::reference_wrapper<FrictionComp>>& frictionComp,
    const std::optional<std::reference_wrapper<GravityComp>>& gravityComp,
    float deltaTime) {
    physics.velocity += physics.acceleration * deltaTime;

    if(frictionComp) {
        FrictionComp& friction = frictionComp->get();
        physics.velocity.x *= std::exp(-friction.coefficient * deltaTime);
        physics.velocity.z *= std::exp(-friction.coefficient * deltaTime);
    }

    if(gravityComp) {
        GravityComp& gravity = gravityComp->get();
        physics.velocity.y += -gravity.coefficient * deltaTime;
    }
}

void PhysicsSys::handleSphereEntityCollision(
    PositionComp& position, SpherePhysicsComp& physics,
    const std::optional<std::reference_wrapper<FrictionComp>>& frictionComp,
    const std::optional<std::reference_wrapper<GravityComp>>& gravityComp,
    float deltaTime) {
    updatePhysics(physics, frictionComp, gravityComp, deltaTime);

    // Predicted new position
    glm::vec3 newPosition =
        position.coords + physics.positionOffset + physics.velocity * deltaTime;

    for(const auto& [otherPhysics] : EntityFilter<BoxPhysicsComp>()) {
        // Expand AABB by the sphere's radius
        glm::vec3 expandedMin = otherPhysics.minCorner - glm::vec3(physics.radius);
        glm::vec3 expandedMax = otherPhysics.maxCorner + glm::vec3(physics.radius);

        // Check if the new position is inside the AABB
        bool colliding =
            newPosition.x >= expandedMin.x && newPosition.x <= expandedMax.x &&
            newPosition.y >= expandedMin.y && newPosition.y <= expandedMax.y &&
            newPosition.z >= expandedMin.z && newPosition.z <= expandedMax.z;
        if(colliding) {
            physics.currentCollision.colliding = true;
            otherPhysics.currentCollision.colliding = true;
        } else {
            continue;
        }

        // Compute penetration depth along each axis
        glm::vec3 penetrationDepth =
            glm::vec3(std::min(std::abs(newPosition.x - expandedMin.x),
                               std::abs(newPosition.x - expandedMax.x)),
                      std::min(std::abs(newPosition.y - expandedMin.y),
                               std::abs(newPosition.y - expandedMax.y)),
                      std::min(std::abs(newPosition.z - expandedMin.z),
                               std::abs(newPosition.z - expandedMax.z)));

        // Resolve the collision along the smallest penetration axis
        if(penetrationDepth.x < penetrationDepth.y &&
           penetrationDepth.x < penetrationDepth.z) {
            // Collision along the X-axis
            if(newPosition.x < (expandedMin.x + expandedMax.x) / 2) {
                physics.currentCollision.xPos = true;
                otherPhysics.currentCollision.xNeg = true;
                newPosition.x = expandedMin.x;
                physics.velocity.x = glm::min(otherPhysics.maxCornerVelocity.x, physics.velocity.x);
            } else {
                physics.currentCollision.xNeg = true;
                otherPhysics.currentCollision.xPos = true;
                newPosition.x = expandedMax.x;
                physics.velocity.x = glm::max(otherPhysics.minCornerVelocity.x, physics.velocity.x);
            }
        } else if(penetrationDepth.y < penetrationDepth.z) {
            // Collision along the Y-axis
            if(newPosition.y < (expandedMin.y + expandedMax.y) / 2) {
                physics.currentCollision.yPos = true;
                otherPhysics.currentCollision.yNeg = true;
                newPosition.y = expandedMin.y;
                physics.velocity.y = glm::min(otherPhysics.maxCornerVelocity.y, physics.velocity.y);
            } else {
                physics.currentCollision.yNeg = true;
                otherPhysics.currentCollision.yPos = true;
                newPosition.y = expandedMax.y;
                physics.velocity.y = glm::max(otherPhysics.minCornerVelocity.y, physics.velocity.y);
            }
        } else {
            // Collision along the Z-axis
            if(newPosition.z < (expandedMin.z + expandedMax.z) / 2) {
                physics.currentCollision.zPos = true;
                otherPhysics.currentCollision.zNeg = true;
                newPosition.z = expandedMin.z;
                physics.velocity.z = glm::min(otherPhysics.maxCornerVelocity.z, physics.velocity.z);
            } else {
                physics.currentCollision.zNeg = true;
                otherPhysics.currentCollision.zPos = true;
                newPosition.z = expandedMax.z;
                physics.velocity.z = glm::max(otherPhysics.minCornerVelocity.z, physics.velocity.z);
            }
        }
    }

    // Apply updated position
    position.coords = newPosition - physics.positionOffset;
    if(gravityComp) {
        GravityComp& gravity = gravityComp->get();
        if(physics.currentCollision.yNeg) {
            // Reset vertical velocity when colliding with the ground
            physics.velocity.y = glm::max(physics.velocity.y, 0.0f);
        }
    }
}

// Draw all collision shapes
void PhysicsSys::drawCollisionShapes() {
    auto drawBox = [this](const glm::vec3& minCorner, const glm::vec3& maxCorner,
                          const glm::mat4& transform, const glm::vec3& color) {
        std::vector<glm::vec3> vertices = {// Bottom face
                                           minCorner,
                                           {maxCorner.x, minCorner.y, minCorner.z},
                                           {maxCorner.x, minCorner.y, minCorner.z},
                                           {maxCorner.x, maxCorner.y, minCorner.z},
                                           {maxCorner.x, maxCorner.y, minCorner.z},
                                           {minCorner.x, maxCorner.y, minCorner.z},
                                           {minCorner.x, maxCorner.y, minCorner.z},
                                           minCorner,

                                           // Top face
                                           {minCorner.x, minCorner.y, maxCorner.z},
                                           {maxCorner.x, minCorner.y, maxCorner.z},
                                           {maxCorner.x, minCorner.y, maxCorner.z},
                                           {maxCorner.x, maxCorner.y, maxCorner.z},
                                           {maxCorner.x, maxCorner.y, maxCorner.z},
                                           {minCorner.x, maxCorner.y, maxCorner.z},
                                           {minCorner.x, maxCorner.y, maxCorner.z},
                                           {minCorner.x, minCorner.y, maxCorner.z},

                                           // Vertical edges
                                           minCorner,
                                           {minCorner.x, minCorner.y, maxCorner.z},
                                           {maxCorner.x, minCorner.y, minCorner.z},
                                           {maxCorner.x, minCorner.y, maxCorner.z},
                                           {minCorner.x, maxCorner.y, minCorner.z},
                                           {minCorner.x, maxCorner.y, maxCorner.z},
                                           {maxCorner.x, maxCorner.y, minCorner.z},
                                           {maxCorner.x, maxCorner.y, maxCorner.z}};

        for(auto& vertex : vertices) {
            vertex = glm::vec3(transform * glm::vec4(vertex, 1.0f));
        }
        RenderingSys::get().addDebugShape(
            vertices, std::vector<glm::vec3>(vertices.size(), color), GL_LINES);
    };

    // Boxes
    EntityFilter<PositionComp, RenderableComp, BoxPhysicsComp> boxEntites;
    for(const auto& [position, renderable, physics] : boxEntites) {
        const auto& obb = renderable.objectResource->boundingBox;

        drawBox(obb->minCorner, obb->maxCorner, position.getTransform(),
                {0.53f, 0.53f, 1.0f});
        drawBox(physics.minCorner, physics.maxCorner, glm::mat4(1.0f),
                {1.0f, physics.currentCollision.colliding ? 1.0f : 0.0f, 0.0f});

        // Draw velocity vectors
        RenderingSys::get().addDebugShape(
            {physics.minCorner, physics.minCornerVelocity + physics.minCorner},
            std::vector<glm::vec3>(2, {0, 1, 0.5}), GL_LINES);
        RenderingSys::get().addDebugShape(
            {physics.maxCorner, physics.maxCornerVelocity + physics.maxCorner},
            std::vector<glm::vec3>(2, {0, 1, 0.5}), GL_LINES);
    }

    // Spheres
    EntityFilter<PositionComp, RenderableComp, SpherePhysicsComp> sphereEntities;
    for(const auto& [position, renderable, physics] : sphereEntities) {
        const auto& debugPoint = Utils::generateSphere(
            physics.radius, position.coords + physics.positionOffset);

        for(const auto& circle : debugPoint) {
            RenderingSys::get().addDebugShape(
                circle,
                std::vector<glm::vec3>(
                    circle.size(),
                    {1.0f, physics.currentCollision.colliding ? 1.0f : 0.0f, 0.0f}),
                GL_LINE_LOOP);
        }
    }
}
