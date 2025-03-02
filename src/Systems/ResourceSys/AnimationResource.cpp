#include "AnimationResource.hpp"

#include "Log.hpp"

AnimationResource::AnimationResource(const std::string &name,
                                     const std::filesystem::path &filePath)
    : mName(name), mFilePath(filePath) {
    if(!loadGLTFModel()) {
        Log::error() << "Failed to load GLTF model from file: " << filePath.string();
    }
}

bool AnimationResource::loadGLTFModel() {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret = false;
    std::string extension = mFilePath.extension().string();
    if(extension == ".glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, mFilePath.string());
    } else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, mFilePath.string());
    }

    if(!warn.empty()) {
        Log::warn() << "GLTF Loader Warning: " << warn;
    }
    if(!err.empty()) {
        Log::error() << "GLTF Loader Error: " << err;
    }

    if(ret) {
        loadNodes(model);
        loadAnimationData(model);
    }

    return ret;
}

void AnimationResource::loadNodes(const tinygltf::Model &model) {
    mNodes.resize(model.nodes.size());

    for(size_t i = 0; i < model.nodes.size(); i++) {
        const tinygltf::Node &gltfNode = model.nodes[i];
        Node &node = mNodes[i];

        // Default values
        node.translation = glm::vec3(0.0f);
        node.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        node.scale = glm::vec3(1.0f);

        // Load translation
        if(!gltfNode.translation.empty()) {
            node.translation = glm::vec3(gltfNode.translation[0], gltfNode.translation[1],
                                         gltfNode.translation[2]);
        }

        // Load rotation
        if(!gltfNode.rotation.empty()) {
            node.rotation = glm::quat(gltfNode.rotation[3], // w
                                      gltfNode.rotation[0], // x
                                      gltfNode.rotation[1], // y
                                      gltfNode.rotation[2]  // z
            );
        }

        // Load scale
        if(!gltfNode.scale.empty()) {
            node.scale =
                glm::vec3(gltfNode.scale[0], gltfNode.scale[1], gltfNode.scale[2]);
        }
    }
}

void AnimationResource::loadAnimationData(const tinygltf::Model &model) {
    for(const auto &anim : model.animations) {
        for(size_t i = 0; i < anim.channels.size(); i++) {
            const tinygltf::AnimationChannel &channel = anim.channels[i];
            const tinygltf::AnimationSampler &sampler = anim.samplers[channel.sampler];

            AnimationChannel animChannel;
            animChannel.targetNode = channel.target_node;
            animChannel.targetPath = channel.target_path;

            // Load timestamps
            const tinygltf::Accessor &input = model.accessors[sampler.input];
            const tinygltf::BufferView &inputView = model.bufferViews[input.bufferView];
            const tinygltf::Buffer &inputBuffer = model.buffers[inputView.buffer];
            const float *timeData =
                reinterpret_cast<const float *>(&inputBuffer.data[inputView.byteOffset]);

            for(size_t j = 0; j < input.count; j++) {
                animChannel.sampler.timestamps.push_back(timeData[j]);
            }

            // Load values (translation, rotation, scale)
            const tinygltf::Accessor &output = model.accessors[sampler.output];
            const tinygltf::BufferView &outputView = model.bufferViews[output.bufferView];
            const tinygltf::Buffer &outputBuffer = model.buffers[outputView.buffer];
            const float *valueData = reinterpret_cast<const float *>(
                &outputBuffer.data[outputView.byteOffset]);

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

            mAnimationChannels.push_back(animChannel);
        }
    }

    // Calculate duration
    for(const auto &channel : mAnimationChannels) {
        float channelDuration = channel.sampler.timestamps.back();
        if(channelDuration > mDuration) {
            mDuration = channelDuration;
        }
    }
}
