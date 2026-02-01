#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <vector>

namespace Utils {
inline bool floatsEqualish(float a, float b, float margin = 0.0001) {
    return std::fabs(a - b) < margin;
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
        float angle = 2.0f * static_cast<float>(std::numbers::pi) * static_cast<float>(i) / static_cast<float>(segments);
        glm::vec3 point = center + radius * (std::cos(angle) * axis1 + std::sin(angle) * axis2);
        circlePoints.push_back(point);
    }

    return circlePoints;
}

// Returns a vector of circles, which can be drawn as line loops to form a sphere
inline std::vector<std::vector<glm::vec3>> generateSphere(float radius, const glm::vec3& center, int latSegments = 16, int longSegments = 16) {
    std::vector<std::vector<glm::vec3>> sphereCircles;

    // Generate latitude circles (XZ plane, varying Y)
    for(int i = 0; i <= latSegments; ++i) {
        float theta = static_cast<float>(std::numbers::pi) * static_cast<float>(i) / static_cast<float>(latSegments); // From 0 to π
        float y = radius * std::cos(theta);
        float latRadius = radius * std::sin(theta);

        std::vector<glm::vec3> latitudeCircle = generateCircle(latRadius, center + glm::vec3(0, y, 0), glm::vec3(0, 1, 0), longSegments);
        sphereCircles.push_back(latitudeCircle);
    }

    // Generate longitude circles (YZ plane, varying X)
    for(int i = 0; i <= longSegments; ++i) {
        float phi = 2.0f * static_cast<float>(std::numbers::pi) * static_cast<float>(i) / static_cast<float>(longSegments); // 0 to 2π
        float x = radius/2 * std::cos(phi);
        float z = radius/2 * std::sin(phi);

        std::vector<glm::vec3> longitudeCircle = generateCircle(radius, center, glm::vec3(std::cos(phi), 0, std::sin(phi)), latSegments);
        sphereCircles.push_back(longitudeCircle);
    }

    return sphereCircles;
}


} // namespace Utils
