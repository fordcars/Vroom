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
#include "Skeleton.hpp"

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

    Skeleton::Ptr getSkeleton(int gltfNodeIndex) {
        auto it = mGltfNodeIndexToSkeleton.find(gltfNodeIndex);
        return it == mGltfNodeIndexToSkeleton.end() ? nullptr : it->second;
    }

    std::vector<Skeleton::Ptr>& getSkeletons() { return mSkeletons; }
    const std::vector<Skeleton::Ptr>& getSkeletons() const { return mSkeletons; }

private:
    std::vector<AnimationNode::Ptr> mNodes;
    std::unordered_set<int> mVisitedInputNodes;
    std::unordered_map<int, AnimationNode*> mGltfNodeIndexToNode;
    std::vector<Skeleton::Ptr> mSkeletons; // All skeletons
    std::unordered_map<int, Skeleton::Ptr> mGltfNodeIndexToSkeleton;
    std::unordered_map<std::string, Animation::Ptr>
        mAnimations; // Animations mapped by name

    void loadNodes(const tinygltf::Model& model, int parentNodeIndex);
    void loadSkeleton(const tinygltf::Model& model, int nodeIndex);
    void loadAnimations(const tinygltf::Model& model);
};
