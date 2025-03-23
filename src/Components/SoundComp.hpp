#pragma once

#include <glm/glm.hpp>
#include "Systems/ResourceSys/ResourceSys.hpp"

struct SoundComp {
    AudioResource::Ptr audioResource;
    glm::vec3 direction {}; // Only used if cone angles are set

    // If angles set to 0, the sound is omnidirectional
    float coneInnerAngle = 0.0f; // In radians
    float coneOuterAngle = 0.0f; // In radians
    float coneOuterGain = 20.0f;
    float rolloffFactor = 2.0f; // How fast the sound fades with distance
};
