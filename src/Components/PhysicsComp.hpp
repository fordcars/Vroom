#pragma once

#include "Systems/ResourceSys/Obj/ObjOrientedBoundingBox.hpp"

struct PhysicsComp {
    glm::vec3 velocity = {};
    glm::vec3 acceleration = {};
};

struct PointPhysicsComp : public PhysicsComp {};
struct BoxPhysicsComp : public PhysicsComp {};
struct SpherePhysicsComp : public PhysicsComp {
    float radius = 1.0f;
};
