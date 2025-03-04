#pragma once

#include <tiny_gltf.h>

#include <filesystem>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

#include "AnimationNode.hpp"
#include "Constants.hpp"

class AnimationContainer;
class Animation {
public:
    using Ptr = std::shared_ptr<Animation>;
    using CPtr = std::shared_ptr<const Animation>;

    enum class TargetPath { Translation, Rotation, Scale };

    struct AnimationSampler {
        std::vector<float> timestamps;
        std::vector<glm::vec4> values; // Quaternion for rotation, vec3 for position/scale
        std::string interpolation;
    };

    struct AnimationChannel {
        AnimationNode* targetNode = nullptr;
        TargetPath targetPath = TargetPath::Translation;
        AnimationSampler sampler;
    };

    static Ptr create(AnimationContainer& container, const tinygltf::Model& model,
                      const tinygltf::Animation& animation) {
        return std::make_shared<Animation>(container, model, animation);
    }

    Animation(AnimationContainer& container, const tinygltf::Model& model,
              const tinygltf::Animation& animation);

    const std::string& getName() const { return mName; }

    std::vector<AnimationChannel>& getChannels() { return mChannels; }

    const std::vector<AnimationChannel>& getChannels() const { return mChannels; }

    float getDuration() const { return mDuration; }

private:
    std::string mName;
    std::vector<AnimationChannel> mChannels;
    float mDuration{};

    void loadAnimationData(AnimationContainer& container, const tinygltf::Model& model,
                           const tinygltf::Animation& animation);
};
