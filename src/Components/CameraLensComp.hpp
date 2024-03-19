#pragma once
#include <glm/glm.hpp>

struct CameraLensComp {
    // w = 1 for lookAt target position, 0 for direction vector
    glm::vec4 direction = {};
    glm::vec3 upVector = {};
    float horizFOV = 0;
    float aspectRatio = 0;
    float nearClippingPlane = 0;
    float farClippingPlane = 0;
};