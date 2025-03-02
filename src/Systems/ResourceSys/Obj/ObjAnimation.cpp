#include "ObjAnimation.hpp"

#include <glm/gtc/type_ptr.hpp> // Required for glm::value_ptr

#include "Log.hpp"

ObjAnimation::ObjAnimation(const std::filesystem::path &filePath,
                           const ObjResource &objResource)
    : mFilePath(filePath) {
    if(!loadGLTFModel(objResource)) {
        Log::error() << "Failed to load GLTF model from file: " << filePath.string();
    }
}

void ObjAnimation::updateBoneBuffer() {
    for(size_t i = 0; i < mBones.size(); ++i) {
        Bone &bone = mBones[i];
        int nodeIndex = bone.id;
        Node &node = mNodes[nodeIndex]; // Get latest node transform

        // Start with identity matrix
        glm::mat4 boneTransform = glm::mat4(1.0f);

        // Apply translation
        boneTransform = glm::translate(boneTransform, node.translation);

        // Apply rotation (convert quaternion to matrix)
        boneTransform *= glm::mat4_cast(node.rotation);

        // Apply scale
        boneTransform = glm::scale(boneTransform, node.scale);

        // Compute final bone transformation
        bone.finalTransform = boneTransform * bone.inverseBindMatrix;

        // If i < max bones, update buffer
        if(i < Constants::MAX_BONES) {
            mBoneTransforms[i] = bone.finalTransform;
        }
    }

    // Load in uniform buffer
    mBoneTransformsBuffer.setData(GL_UNIFORM_BUFFER, mBoneTransforms);
}

bool ObjAnimation::loadGLTFModel(const ObjResource &objResource) {
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
        loadBones(model);
        loadVertices(objResource, model);
    }

    return ret;
}

void ObjAnimation::loadNodes(const tinygltf::Model &model) {
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

void ObjAnimation::loadAnimationData(const tinygltf::Model &model) {
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

void ObjAnimation::loadBones(const tinygltf::Model &model) {
    if(model.skins.empty()) {
        Log::error() << "No skeleton found in GLTF model!";
        return;
    }

    const tinygltf::Skin &skin = model.skins[0]; // Assume first skin
    mBones.resize(skin.joints.size());

    // Load inverse bind matrices
    std::vector<glm::mat4> inverseBindMatrices;
    if(skin.inverseBindMatrices >= 0) {
        const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        const float *matrixData =
            reinterpret_cast<const float *>(&buffer.data[bufferView.byteOffset]);

        for(size_t i = 0; i < accessor.count; i++) {
            glm::mat4 mat;
            std::memcpy(glm::value_ptr(mat), matrixData + i * 16,
                        sizeof(glm::mat4)); // Copy 16 floats
            inverseBindMatrices.push_back(mat);
        }
    }

    // Load bones
    for(size_t i = 0; i < skin.joints.size(); i++) {
        int nodeIndex = skin.joints[i];

        mBones[i].id = nodeIndex;
        mBones[i].name = model.nodes[nodeIndex].name;
        mBones[i].inverseBindMatrix =
            inverseBindMatrices.empty() ? glm::mat4(1.0f) : inverseBindMatrices[i];
        mBones[i].finalTransform = glm::mat4(1.0f);
    }

    // Check if we bust bone limit
    if(mBones.size() > Constants::MAX_BONES) {
        Log::warn() << "Bone count exceeds maximum limit of " << Constants::MAX_BONES;
    }
}

void ObjAnimation::loadVertices(const ObjResource &objResource,
                                const tinygltf::Model &model) {
    for(const auto &mesh : model.meshes) {
        loadVertices(objResource, model, mesh);
    }
}

void loadVertices(const ObjResource &objResource, const tinygltf::Model &model,
                  const tinygltf::Mesh &mesh) {
    Log::debug() << "Loading vertices for mesh: " << mesh.name;
    if(mesh.primitives.size() > 1) {
        Log::warn() << "Mesh " << mesh.name
                    << " has multiple primitives, only loading first one!";
    }

    auto &primitive = mesh.primitives[0]; // Assume first primitive

    // Look for JOINTS_0 (bone IDs)
    auto itJoints = primitive.attributes.find("JOINTS_0");
    if(itJoints != primitive.attributes.end()) {
        const tinygltf::Accessor &accessor = model.accessors[itJoints->second];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        const uint16_t *boneData =
            reinterpret_cast<const uint16_t *>(&buffer.data[bufferView.byteOffset]);

        for(size_t i = 0; i < accessor.count; i++) {
            vertices[i].boneIDs = glm::ivec4(boneData[i * 4], boneData[i * 4 + 1],
                                             boneData[i * 4 + 2], boneData[i * 4 + 3]);
        }
    }

    // Look for WEIGHTS_0 (bone weights)
    auto itWeights = primitive.attributes.find("WEIGHTS_0");
    if(itWeights != primitive.attributes.end()) {
        const tinygltf::Accessor &accessor = model.accessors[itWeights->second];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        const float *weightData =
            reinterpret_cast<const float *>(&buffer.data[bufferView.byteOffset]);

        for(size_t i = 0; i < accessor.count; i++) {
            vertices[i].weights = glm::vec4(weightData[i * 4], weightData[i * 4 + 1],
                                            weightData[i * 4 + 2], weightData[i * 4 + 3]);
        }
    }
}
}