#include "ObjAnimation.hpp"

#include <glm/gtc/type_ptr.hpp> // Required for glm::value_ptr
#include <stack>

#include "Log.hpp"

ObjAnimation::ObjAnimation(const tinygltf::Model &model) {
    Log::debug() << "Loading animation data.";
    loadAnimations(model);
}

void ObjAnimation::loadSkeleton(const tinygltf::Model &model, const tinygltf::Skin &skin,
                                std::size_t meshId) {
    // Check if skeleton is already loaded
    if(mNodeToJointIndex.find(skin.skeleton) != mNodeToJointIndex.end()) {
        std::size_t rootNodeIndex = mNodeToJointIndex[skin.skeleton];
        auto it = std::find_if(
            skeletons.begin(), skeletons.end(),
            [rootNodeIndex](const Skeleton &s) { return s.rootNode == rootNodeIndex; });
        if(it != skeletons.end()) {
            Log::debug() << "Skeleton already loaded.";
            meshToSkeleton.insert({meshId, std::distance(skeletons.begin(), it)});
            return;
        }
    }

    auto skeleton = std::make_shared <

                    // Make sure all nodes are loaded
                    loadNodes(model, skin.skeleton);
    skeleton.rootNode = mNodeToJointIndex[skin.skeleton];

    // Load joints
    for(const auto &joint : skin.joints) {
        skeleton.joints.push_back(mNodeToJointIndex[joint]);
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
        skeleton.inverseBindMatrices.push_back(inverseBindMatrix);
    }

    Log::debug() << "Loaded skeleton with " << skeleton.joints.size() << " joints.";
    if(skeleton.joints.size() != skeleton.inverseBindMatrices.size()) {
        Log::warn() << "Mismatch between joint count (" << skeleton.joints.size()
                    << ") and inverse bind matrices count ("
                    << skeleton.inverseBindMatrices.size() << ").";
    }

    skeletons.insert({skeletons.size(), skeleton});
}

void ObjAnimation::updateTransformBuffers() {
    for(auto &[_, skeleton] : skeletons) {
        std::vector<glm::mat4> transforms;

        // Todo: optimize this by only updating the transforms that have changed
        for(std::size_t i = 0; i < skeleton.joints.size(); i++) {
            glm::mat4 transform = glm::mat4(1.0f);
            std::size_t jointIndex = skeleton.joints[i];
            while(true) {
                Node &node = jointNodes[jointIndex];
                transform = glm::translate(transform, node.translation);
                transform *= glm::mat4_cast(node.rotation);
                transform = glm::scale(transform, node.scale);

                if(!node.parent) break;
                jointIndex = *node.parent;
            }
            transform = glm::inverse(transform) * skeleton.inverseBindMatrices[i];
            transforms.push_back(transform);
        }
        skeleton.transformsBuffer.setData(GL_UNIFORM_BUFFER, transforms);
    }
}

void ObjAnimation::loadNodes(const tinygltf::Model &model, int parentNode) {
    std::stack<std::pair<int, std::optional<std::size_t>>>
        stack; // Node index, our parent index
    stack.push({parentNode, std::nullopt});

    while(!stack.empty()) {
        auto [nodeIndex, parentIndex] = stack.top();
        stack.pop();
        if(mAddedNodes.find(nodeIndex) != mAddedNodes.end()) continue;

        const tinygltf::Node &node = model.nodes[nodeIndex];

        // Create and add joint node
        Node jointNode;
        jointNode.parent = parentIndex;
        jointNode.translation = node.translation.empty()
                                    ? glm::vec3(0.0f)
                                    : glm::vec3(node.translation[0], node.translation[1],
                                                node.translation[2]);
        jointNode.rotation = node.rotation.empty()
                                 ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
                                 : glm::quat(node.rotation[3], node.rotation[0],
                                             node.rotation[1], node.rotation[2]);
        jointNode.scale = node.scale.empty()
                              ? glm::vec3(1.0f)
                              : glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
        jointNodes.push_back(jointNode);

        std::size_t newJointIndex = jointNodes.size() - 1;
        mNodeToJointIndex.insert({nodeIndex, newJointIndex});

        for(int childIndex : node.children) {
            stack.push({childIndex, newJointIndex});
        }
        mAddedNodes.insert(nodeIndex);
    }
}

void ObjAnimation::loadAnimations(const tinygltf::Model &model) {
    for(const auto &anim : model.animations) {
        std::vector<AnimationChannel> channels;
        Log::debug() << "Loading animation '" << anim.name << "'.";

        for(size_t i = 0; i < anim.channels.size(); i++) {
            const tinygltf::AnimationChannel &channel = anim.channels[i];
            const tinygltf::AnimationSampler &sampler = anim.samplers[channel.sampler];

            AnimationChannel animChannel;
            animChannel.targetNode = mNodeToJointIndex[channel.target_node];
            animChannel.targetPath =
                channel.target_path == "translation" ? TargetPath::Translation
                : channel.target_path == "rotation"  ? TargetPath::Rotation
                                                     : TargetPath::Scale;

            // Load timestamps
            const tinygltf::Accessor &input = model.accessors[sampler.input];
            const tinygltf::BufferView &inputView = model.bufferViews[input.bufferView];
            const tinygltf::Buffer &inputBuffer = model.buffers[inputView.buffer];
            const float *timeData = reinterpret_cast<const float *>(
                &inputBuffer.data[inputView.byteOffset + input.byteOffset]);

            for(size_t j = 0; j < input.count; j++) {
                animChannel.sampler.timestamps.push_back(timeData[j]);
            }

            // Load values (translation, rotation, scale)
            const tinygltf::Accessor &output = model.accessors[sampler.output];
            const tinygltf::BufferView &outputView = model.bufferViews[output.bufferView];
            const tinygltf::Buffer &outputBuffer = model.buffers[outputView.buffer];
            const float *valueData = reinterpret_cast<const float *>(
                &outputBuffer.data[outputView.byteOffset + output.byteOffset]);

            for(size_t j = 0; j < output.count; j++) {
                if(channel.target_path == "rotation") {
                    animChannel.sampler.values.push_back(
                        glm::vec4(valueData[j * 4], valueData[j * 4 + 1],
                                  valueData[j * 4 + 2], valueData[j * 4 + 3]));
                } else {
                    animChannel.sampler.values.push_back(
                        glm::vec4(valueData[j * 3], valueData[j * 3 + 1],
                                  valueData[j * 3 + 2], 0.0f));
                }
            }

            channels.push_back(animChannel);
        }

        // Calculate duration
        float maxTime = 0.0f;
        for(const auto &channel : channels) {
            float time = channel.sampler.timestamps.back();
            if(time > maxTime) maxTime = time;
        }
        animations.emplace(std::pair{anim.name, Animation{anim.name, channels, maxTime}});
    }
}
