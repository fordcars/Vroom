#pragma once

#include <tiny_gltf.h>

#include <filesystem>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "AnimationNode.hpp"
#include "Constants.hpp"
#include "Systems/ResourceSys/Obj/GPUBuffer.hpp"

class AnimationContainer;
class Node;
class Skeleton {
public:
    using Ptr = std::shared_ptr<Skeleton>;
    using CPtr = std::shared_ptr<const Skeleton>;

    static Ptr create(AnimationContainer& container, const tinygltf::Model& model,
                      int nodeIndex) {
        return std::make_shared<Skeleton>(container, model, nodeIndex);
    }

    Skeleton(AnimationContainer& container, const tinygltf::Model& model, int nodeIndex);
    void updateTransformBuffer();

    const std::vector<AnimationNode*>& getJoints() const { return mJoints; }
    const GPUBuffer& getTransformBuffer() const { return mTransformBuffer; }

private:
    std::vector<AnimationNode*> mJoints;         // Nodes in the skeleton
    std::vector<glm::mat4> mInverseBindMatrices; // Inverse bind matrices for each joint
    GPUBuffer mTransformBuffer;                  // Transform matrices for each joint

    void load(AnimationContainer& container, const tinygltf::Model& model,
              int skinNodeIndex);
};
