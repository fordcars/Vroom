#pragma once

#include <tiny_gltf.h>

#include <filesystem>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Constants.hpp"
#include "Systems/ResourceSys/Obj/GPUBuffer.hpp"

class ObjAnimation {
public:
    using Ptr = std::shared_ptr<ObjAnimation>;
    using CPtr = std::shared_ptr<const ObjAnimation>;

    struct AnimationSampler {
        std::vector<float> timestamps;
        std::vector<glm::vec4> values; // Quaternion for rotation, vec3 for position/scale
        std::string interpolation;
    };

    struct AnimationChannel {
        int targetNode;
        std::string targetPath; // "translation", "rotation", "scale"
        AnimationSampler sampler;
    };

    struct Node {
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
    };

    struct Bone {
        int id;
        std::string name;
        glm::mat4 inverseBindMatrix;
        glm::mat4 finalTransform;
    };

    float duration;
    std::vector<Node> nodes; // Store all nodes (skeleton + objects)
    std::vector<AnimationChannel> animationChannels;
    std::vector<Bone> bones;
    std::vector<glm::mat4> boneTransforms{Constants::MAX_BONES, glm::mat4(1.0f)};
    GPUBuffer boneTransformsBuffer;

    static Ptr create() { return std::make_shared<ObjAnimation>(); }

    ObjAnimation();
    void updateBoneBuffer();

private:
    void loadNodes(const tinygltf::Model& model);
    void loadAnimationData(const tinygltf::Model& model);
    void loadBones(const tinygltf::Model& model);
};
