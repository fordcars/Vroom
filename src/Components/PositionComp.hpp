#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct PositionComp {
    glm::vec3 coords = {};
    glm::vec3 rotation = {};
    glm::vec3 scale = {1, 1, 1};

    glm::mat4 getTransform() const {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), coords);
        transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        transform = glm::scale(transform, scale);
        return transform;
    }
};