#include "Skeleton.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // Required for glm::value_ptr
#include <stack>

#include "AnimationContainer.hpp"
#include "AnimationNode.hpp"
#include "Log.hpp"

Skeleton::Skeleton(AnimationContainer &container, const tinygltf::Model &model,
                   int nodeIndex) {
    load(container, model, nodeIndex);
}

void Skeleton::updateTransformBuffer() {
    std::vector<glm::mat4> transforms;

    // Todo: optimize this by only updating the transforms that have changed
    for(std::size_t i = 0; i < mJoints.size(); i++) {
        glm::mat4 transform = glm::mat4(1.0f);
        AnimationNode *joint = mJoints[i];
        while(joint != nullptr) {
            transform = glm::translate(transform, joint->translation);
            transform *= glm::mat4_cast(joint->rotation);
            transform = glm::scale(transform, joint->scale);
            joint = joint->parent;
        }
        transform = glm::inverse(transform) * mInverseBindMatrices[i];
        transforms.push_back(transform);
    }
    mTransformBuffer.setData(GL_UNIFORM_BUFFER, transforms);
}

void Skeleton::load(AnimationContainer &container, const tinygltf::Model &model,
                    int nodeIndex) {
    const tinygltf::Node &node = model.nodes[nodeIndex];

    if(node.skin < 0) {
        Log::debug() << "No skeleton data found!";
        return;
    } else {
        Log::debug() << "Loading skeleton data.";
    }

    const tinygltf::Skin &skin = model.skins[node.skin];
    mRootNode = container.getNode(nodeIndex);

    // Load joints
    for(const auto &joint : skin.joints) {
        mJoints.push_back(container.getNode(joint));
    }

    // Load inverse bind matrices
    const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
    const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
    const float *data = reinterpret_cast<const float *>(
        &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

    for(size_t i = 0; i < accessor.count; i++) {
        glm::mat4 inverseBindMatrix;
        for(size_t j = 0; j < 16; j++) {
            inverseBindMatrix[j / 4][j % 4] = data[i * 16 + j];
        }
        mInverseBindMatrices.push_back(inverseBindMatrix);
    }

    Log::debug() << "Loaded skeleton with " << mJoints.size() << " joints.";
    if(mJoints.size() != mInverseBindMatrices.size()) {
        Log::warn() << "Mismatch between joint count (" << mJoints.size()
                    << ") and inverse bind matrices count ("
                    << mInverseBindMatrices.size() << ").";
    }
}
