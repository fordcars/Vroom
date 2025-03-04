#include "AnimationContainer.hpp"

#include <stack>

#include "Animation.hpp"
#include "Log.hpp"

AnimationContainer::AnimationContainer(const tinygltf::Model &model) {
    if(model.scenes.empty()) {
        Log::warn() << "No scenes found in the model.";
    } else {
        for(auto &node : model.scenes[0].nodes) {
            loadNodes(model, node);
        }
        loadAnimations(model);
    }
}

// Loads all nodes under the given parent node
void AnimationContainer::loadNodes(const tinygltf::Model &model, int parentNodeIndex) {
    if(mVisitedInputNodes.find(parentNodeIndex) != mVisitedInputNodes.end()) {
        Log::debug() << "Node " << parentNodeIndex << " already visited.";
        return;
    }
    if(parentNodeIndex < 0) {
        Log::debug() << "Parent node index is negative.";
        return;
    }
    Log::debug() << "Loading nodes from parent node " << parentNodeIndex << ".";

    std::stack<std::pair<int, AnimationNode *>>
        stack; // Input node index, our parent node
    stack.push({parentNodeIndex, nullptr});

    while(!stack.empty()) {
        auto [nodeIndex, parentNode] = stack.top();
        stack.pop();
        if(mVisitedInputNodes.find(nodeIndex) != mVisitedInputNodes.end()) continue;

        const tinygltf::Node &node = model.nodes[nodeIndex];

        // Create and add joint node
        AnimationNode::Ptr newNode = std::make_unique<AnimationNode>();
        AnimationNode *newNodePtr = newNode.get();
        newNode->parent = parentNode;
        newNode->translation = node.translation.empty()
                                   ? glm::vec3(0.0f)
                                   : glm::vec3(node.translation[0], node.translation[1],
                                               node.translation[2]);
        newNode->rotation = node.rotation.empty()
                                ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
                                : glm::quat(node.rotation[3], node.rotation[0],
                                            node.rotation[1], node.rotation[2]);
        newNode->scale = node.scale.empty()
                             ? glm::vec3(1.0f)
                             : glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

        if(parentNode)
            parentNode->children.push_back(newNodePtr); // Add as child to parent
        mGltfNodeIndexToNode.insert({nodeIndex, newNodePtr});
        mNodes.push_back(std::move(newNode));

        for(int childIndex : node.children) {
            stack.push({childIndex, newNodePtr});
        }
        mVisitedInputNodes.insert(nodeIndex);

        if(node.skin >= 0) {
            loadSkeleton(model, nodeIndex);
        }
    }
}

void AnimationContainer::loadSkeleton(const tinygltf::Model &model, int nodeIndex) {
    const tinygltf::Node &node = model.nodes[nodeIndex];
    Skeleton::Ptr skeleton = Skeleton::create(*this, model, nodeIndex);
    mSkeletons.push_back(skeleton);
    mGltfSkinIndexToSkeleton.insert({node.skin, skeleton});
}

void AnimationContainer::loadAnimations(const tinygltf::Model &model) {
    for(const auto &anim : model.animations) {
        Animation::Ptr animation = Animation::create(*this, model, anim);
        mAnimations.insert({anim.name, animation});
    }
}