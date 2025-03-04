#include "Skin.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // Required for glm::value_ptr
#include <stack>

#include "AnimationContainer.hpp"
#include "AnimationNode.hpp"
#include "Log.hpp"

Skin::Skin(AnimationContainer &container, const tinygltf::Model &model,
           const tinygltf::Skin &skin) {
    load(container, model, skin);
    updateTransformBuffer();
}

void Skin::updateTransformBuffer() {
    std::vector<glm::mat4> outTransforms;
    std::unordered_map<AnimationNode *, glm::mat4> cachedTransforms;
    outTransforms.reserve(mJoints.size());

    // Apply all joints transformations, from root to joint[i]
    for(std::size_t i = 0; i < mJoints.size(); i++) {
        std::stack<AnimationNode *> stack;
        glm::mat4 accumulatedTransform = glm::mat4(1.0f);
        AnimationNode *joint = mJoints[i];

        // Go up the hierarchy
        // If we hit a joint that has already been processed, we can use its cached
        // transform
        while(joint != nullptr) {
            if(cachedTransforms.find(joint) != cachedTransforms.end()) {
                accumulatedTransform = cachedTransforms[joint];
                break;
            }

            stack.push(joint);
            joint = joint->parent;
        }

        // Apply from root (or after last cached) to joint
        while(!stack.empty()) {
            joint = stack.top();
            stack.pop();

            glm::mat4 localTransform = glm::mat4(1.0f);
            localTransform = glm::translate(glm::mat4(1.0f), joint->translation) *
                             glm::mat4_cast(joint->rotation) *
                             glm::scale(glm::mat4(1.0f), joint->scale);

            accumulatedTransform = accumulatedTransform * localTransform;
            cachedTransforms.insert({joint, accumulatedTransform});
            joint = joint->parent;
        }

        accumulatedTransform = accumulatedTransform * mInverseBindMatrices[i];
        outTransforms.push_back(accumulatedTransform);
    }
    mTransformBuffer.setData(GL_UNIFORM_BUFFER, outTransforms);
}

void Skin::load(AnimationContainer &container, const tinygltf::Model &model,
                const tinygltf::Skin &skin) {
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

    Log::debug() << "Loaded skin with " << mJoints.size() << " joints.";
    if(mJoints.size() != mInverseBindMatrices.size()) {
        Log::warn() << "Mismatch between joint count (" << mJoints.size()
                    << ") and inverse bind matrices count ("
                    << mInverseBindMatrices.size() << ").";
    }
}
