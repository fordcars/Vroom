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

class ObjResource;
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

    static Ptr create(const std::filesystem::path& filePath,
                      const ObjResource& objResource) {
        return std::make_shared<ObjAnimation>(filePath, const ObjResource& objResource);
    }

    ObjAnimation(const std::filesystem::path& filePath, const ObjResource& objResource);

    float getDuration() { return mDuration; }

    std::vector<Node>& getNodes() { return mNodes; }
    const std::vector<Node>& getNodes() const { return mNodes; }
    std::vector<AnimationChannel>& getAnimationChannels() { return mAnimationChannels; }
    const std::vector<AnimationChannel>& getAnimationChannels() const {
        return mAnimationChannels;
    }

    GPUBuffer& getBoneIdsBuffer() { return mBoneIdsBuffer; }
    const GPUBuffer& getBoneIdsBuffer() const { return mBoneIdsBuffer; }

    GPUBuffer& getBoneWeightsBuffer() { return mBoneWeightsBuffer; }
    const GPUBuffer& getBoneWeightsBuffer() const { return mBoneWeightsBuffer; }

    GPUBuffer& getBoneTransformsUniformBuffer() { return mBoneTransformsBuffer; }
    const GPUBuffer& getBoneTransformsUniformBuffer() const {
        return mBoneTransformsBuffer;
    }

    void updateBoneBuffer();

private:
    bool loadGLTFModel(const ObjResource& objResource);
    void loadNodes(const tinygltf::Model& model);
    void loadAnimationData(const tinygltf::Model& model);
    void loadBones(const tinygltf::Model& model);
    void loadVertices(const ObjResource& objResource, const tinygltf::Model& model);
    void loadVertices(const ObjResource& objResource, const tinygltf::Model& model,
                      const tinygltf::Mesh& mesh);

    std::filesystem::path mFilePath;
    float mDuration;
    std::vector<Node> mNodes; // Store all nodes (skeleton + objects)
    std::vector<AnimationChannel> mAnimationChannels;
    std::vector<Bone> mBones;
    std::vector<glm::mat4> mBoneTransforms{Constants::MAX_BONES, glm::mat4(1.0f)};

    GPUBuffer mBoneIdsBuffer;
    GPUBuffer mBoneWeightsBuffer;
    GPUBuffer mBoneTransformsBuffer;
};
