#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

// In glTF, represents a node in the scene graph
struct AnimationNode {
    using Ptr = std::unique_ptr<AnimationNode>;
    using CPtr = std::unique_ptr<const AnimationNode>;

    AnimationNode* parent = nullptr;
    std::vector<AnimationNode*> children;
    glm::vec3 translation{};
    glm::quat rotation{};
    glm::vec3 scale{};
};
