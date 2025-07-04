#pragma once

#include "Systems/ResourceSys/Obj/ObjBoundingBox.hpp"

struct PhysicsComp {
    struct CurrentCollision {
        bool colliding = false;
        bool yPos = false;
        bool yNeg = false;
        bool xPos = false;
        bool xNeg = false;
        bool zPos = false;
        bool zNeg = false;
    };

    glm::vec3 positionOffset = {};
    glm::vec3 velocity = {};
    glm::vec3 acceleration = {};
    CurrentCollision currentCollision = {};
};

struct NullPhysicsComp : public PhysicsComp {};
struct BoxPhysicsComp : public PhysicsComp {
    // AABB
    glm::vec3 minCorner {};
    glm::vec3 maxCorner {};
    glm::vec3 minCornerVelocity {};
    glm::vec3 maxCornerVelocity {};
};
struct SpherePhysicsComp : public PhysicsComp {
    float radius = 1.0f;
};
