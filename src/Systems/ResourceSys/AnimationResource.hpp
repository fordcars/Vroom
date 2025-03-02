#pragma once

#include <tiny_gltf.h>

#include <filesystem>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

class AnimationResource {
public:
    using Ptr = std::shared_ptr<AnimationResource>;
    using CPtr = std::shared_ptr<const AnimationResource>;

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

    static Ptr create(const std::string& name, const std::filesystem::path& filePath) {
        return std::make_shared<AnimationResource>(name, filePath);
    }

    AnimationResource(const std::string& name, const std::filesystem::path& filePath);

    float getDuration() { return mDuration; }

    std::vector<Node>& getNodes() { return mNodes; }
    const std::vector<Node>& getNodes() const { return mNodes; }
    std::vector<AnimationChannel>& getAnimationChannels() { return mAnimationChannels; }
    const std::vector<AnimationChannel>& getAnimationChannels() const {
        return mAnimationChannels;
    }

private:
    bool loadGLTFModel();
    void loadNodes(const tinygltf::Model& model);
    void loadAnimationData(const tinygltf::Model& model);

    std::string mName;
    std::filesystem::path mFilePath;
    float mDuration;
    std::vector<Node> mNodes; // Store all nodes (skeleton + objects)
    std::vector<AnimationChannel> mAnimationChannels;
};
