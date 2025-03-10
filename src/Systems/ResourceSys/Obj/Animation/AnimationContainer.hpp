#pragma once

#include <tiny_gltf.h>

#include <array>
#include <filesystem>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Animation.hpp"
#include "AnimationNode.hpp"
#include "Constants.hpp"
#include "Log.hpp"
#include "Skin.hpp"

class AnimationContainer {
public:
    using Ptr = std::shared_ptr<AnimationContainer>;
    using CPtr = std::shared_ptr<const AnimationContainer>;

    static Ptr create(const tinygltf::Model& model) {
        return std::make_shared<AnimationContainer>(model);
    }

    AnimationContainer(const tinygltf::Model& model);

    Animation* getAnimation(std::size_t nameIndex) {
        if(nameIndex >= mAnimations.size()) {
            Log::error() << "Animation " << nameIndex << " out of bounds.";
            return nullptr;
        }
        if(!mAnimations[nameIndex]) {
            auto name = Constants::AnimationName::runtimeGet(nameIndex);
            if(name.has_value()) {
                Log::error() << "Animation " << name.value()
                             << " not found in container.";
            }
            return nullptr;
        }
        return mAnimations[nameIndex].get();
    }

    AnimationNode* getNode(int gltfNodeIndex) {
        auto it = mGltfNodeIndexToNode.find(gltfNodeIndex);
        return it == mGltfNodeIndexToNode.end() ? nullptr : it->second;
    }

    const AnimationNode* getNode(int gltfNodeIndex) const {
        auto it = mGltfNodeIndexToNode.find(gltfNodeIndex);
        return it == mGltfNodeIndexToNode.end() ? nullptr : it->second;
    }

    Skin::Ptr getSkin(int gltfSkinIndex) {
        auto it = mGltfSkinIndexToSkin.find(gltfSkinIndex);
        return it == mGltfSkinIndexToSkin.end() ? nullptr : it->second;
    }

    std::vector<Skin::Ptr>& getSkins() { return mSkins; }
    const std::vector<Skin::Ptr>& getSkins() const { return mSkins; }

private:
    std::vector<AnimationNode::Ptr> mNodes;
    std::unordered_set<int> mVisitedInputNodes;
    std::unordered_map<int, AnimationNode*> mGltfNodeIndexToNode;
    std::vector<Skin::Ptr> mSkins; // All skins
    std::unordered_map<int, Skin::Ptr> mGltfSkinIndexToSkin;
    std::array<Animation::Ptr, Constants::AnimationName::size()>
        mAnimations{}; // Animations mapped by name index

    void loadNodes(const tinygltf::Model& model, int parentNodeIndex);
    void loadSkins(const tinygltf::Model& model);
    void loadAnimations(const tinygltf::Model& model);
};
