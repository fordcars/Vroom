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
}

void Skin::updateTransformBuffer() {
    std::vector<glm::mat4> transforms;

    // Apply all joints transformations (and their parents)
    for(std::size_t i = 0; i < mJoints.size(); i++) {
        glm::mat4 transform = glm::mat4(1.0f);
        AnimationNode *joint = mJoints[i];

        while(joint != nullptr) {
            glm::mat4 localTransform = glm::mat4(1.0f);

            localTransform = glm::translate(glm::mat4(1.0f), joint->translation) *
                             glm::mat4_cast(joint->rotation) *
                             glm::scale(glm::mat4(1.0f), joint->scale);

            // Apply from child to root
            transform = localTransform * transform;
            joint = joint->parent;
        }

        transform = transform * mInverseBindMatrices[i];
        transforms.push_back(transform);
    }
    mTransformBuffer.setData(GL_UNIFORM_BUFFER, transforms);
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
