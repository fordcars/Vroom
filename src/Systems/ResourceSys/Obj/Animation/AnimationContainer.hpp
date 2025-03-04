#pragma once

#include <tiny_gltf.h>

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

    Animation* getAnimation(const std::string& name) {
        auto it = mAnimations.find(name);
        if(it == mAnimations.end()) {
            Log::warn() << "Animation '" << name << "' not found.";
            return nullptr;
        }
        return it == mAnimations.end() ? nullptr : it->second.get();
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
    std::unordered_map<std::string, Animation::Ptr>
        mAnimations; // Animations mapped by name

    void loadNodes(const tinygltf::Model& model, int parentNodeIndex);
    void loadSkins(const tinygltf::Model& model);
    void loadAnimations(const tinygltf::Model& model);
};
