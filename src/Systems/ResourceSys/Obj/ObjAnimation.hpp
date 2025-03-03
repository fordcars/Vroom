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

    struct Animation {
        std::string name;
        std::vector<AnimationChannel> channels;
        float duration;
    };

    struct Node {
        int parentIndex;
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

    std::unordered_map<std::string, Animation> animations;
    std::vector<Node> nodes; // Store all nodes (skeleton + objects)
    std::vector<Bone> bones;
    std::vector<glm::mat4> boneTransforms{Constants::MAX_BONES, glm::mat4(1.0f)};
    GPUBuffer boneTransformsBuffer;

    static Ptr create(const tinygltf::Model& model) {
        return std::make_shared<ObjAnimation>(model);
    }

    ObjAnimation(const tinygltf::Model& model);
    void updateBoneBuffer();

private:
    void loadNodes(const tinygltf::Model& model);
    void loadAnimationData(const tinygltf::Model& model);
    void loadBones(const tinygltf::Model& model);
};
