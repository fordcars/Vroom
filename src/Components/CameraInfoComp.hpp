#pragma once
#include <glm/glm.hpp>

struct CameraInfoComp {
    // w = 1 position, 0 for direction vector
    glm::vec4 direction = {};
    glm::vec3 upVector = {};
    float horizFOV = 0;
    float aspectRatio = 0;
    float nearClippingPlane = 0;
    float farClippingPlane = 0;
};