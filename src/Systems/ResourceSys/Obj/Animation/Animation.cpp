#include "Animation.hpp"

#include <stack>

#include "AnimationContainer.hpp"
#include "Log.hpp"

Animation::Animation(AnimationContainer &container, const tinygltf::Model &model,
                     const tinygltf::Animation &animation) {
    loadAnimationData(container, model, animation);
}

void Animation::loadAnimationData(AnimationContainer &container,
                                  const tinygltf::Model &model,
                                  const tinygltf::Animation &animation) {
    Log::debug() << "Loading animation data for '" << animation.name << "'.";
    mName = animation.name;

    for(size_t i = 0; i < animation.channels.size(); i++) {
        const tinygltf::AnimationChannel &channel = animation.channels[i];
        const tinygltf::AnimationSampler &sampler = animation.samplers[channel.sampler];

        AnimationChannel animChannel;
        animChannel.targetNode = container.getNode(channel.target_node);
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

        const unsigned char *dataPtr =
            outputBuffer.data.data() + outputView.byteOffset + output.byteOffset;
        size_t stride =
            (outputView.byteStride == 0)
                ? tinygltf::GetComponentSizeInBytes(output.componentType) * output.type
                : outputView.byteStride;

        for(size_t j = 0; j < output.count; j++) {
            size_t indexOffset = j * stride / sizeof(float);
            const float *valueData =
                reinterpret_cast<const float *>(dataPtr + j * stride);

            if(channel.target_path == "rotation") {
                animChannel.sampler.values.push_back(
                    glm::vec4(valueData[0], valueData[1], valueData[2], valueData[3]));
            } else {
                animChannel.sampler.values.push_back(
                    glm::vec4(valueData[0], valueData[1], valueData[2], 0.0f));
            }
        }

        mChannels.push_back(animChannel);
    }

    // Calculate duration
    mDuration = 0.0f;
    for(const auto &channel : mChannels) {
        float time = channel.sampler.timestamps.back();
        if(time > mDuration) mDuration = time;
    }
}
