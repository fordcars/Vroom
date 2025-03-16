#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <vector>

namespace Utils {
inline bool floatsEqualish(float a, float b, float margin = 0.0001) {
    return fabs(a - b) < margin;
}

inline std::vector<glm::vec3> generateCircle(float radius, const glm::vec3& center,
                                             const glm::vec3& normal, int segments = 32) {
    std::vector<glm::vec3> circlePoints;
    circlePoints.reserve(segments);

    glm::vec3 axis1 = glm::normalize(glm::cross(normal, glm::vec3(1.0f, 0.0f, 0.0f)));
    if(glm::length(axis1) < 0.001f) {
        axis1 = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    glm::vec3 axis2 = glm::normalize(glm::cross(normal, axis1));

    for(int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        glm::vec3 point = center + radius * (cos(angle) * axis1 + sin(angle) * axis2);
        circlePoints.push_back(point);
    }

    return circlePoints;
}

} // namespace Utils
