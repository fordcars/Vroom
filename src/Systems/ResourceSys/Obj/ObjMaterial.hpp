#pragma once

#include <glm/glm.hpp>

// std140-compatible struct
struct ObjMaterial {
    glm::vec3 baseColor;
    float p1;
    glm::vec3 emission;
    float p2;

    float alpha;
    float metallic;
    float roughness;
    float sheen;
};