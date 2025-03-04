#include "GltfLoader.hpp"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <stack>
#include <utility>

#include "Animation/Animation.hpp"
#include "Log.hpp"
#include "ObjMaterial.hpp"
#include "ObjMesh.hpp"
#include "ObjResource.hpp"
#include "WavefrontLoader.hpp"

namespace {
template <typename T>
void extractAttribute(const tinygltf::Primitive& primitive, const tinygltf::Model& model,
                      const std::string& attribute, std::vector<T>& output) {
    auto it = primitive.attributes.find(attribute);
    if(it == primitive.attributes.end()) return;

    const tinygltf::Accessor& accessor = model.accessors[it->second];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    const unsigned char* dataPtr =
        buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
    output.resize(accessor.count);

    if constexpr(std::is_same<T, glm::vec3>::value) {
        const glm::vec3* data = reinterpret_cast<const glm::vec3*>(dataPtr);
        std::copy(data, data + accessor.count, output.begin());
    } else if constexpr(std::is_same<T, glm::vec2>::value) {
        const glm::vec2* data = reinterpret_cast<const glm::vec2*>(dataPtr);
        std::copy(data, data + accessor.count, output.begin());
    } else if constexpr(std::is_same<T, glm::uvec4>::value) {
        if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(dataPtr);
            for(size_t i = 0; i < accessor.count; i++) {
                output[i] = glm::uvec4(data[i * 4 + 0], data[i * 4 + 1], data[i * 4 + 2],
                                       data[i * 4 + 3]);
            }
        } else if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            const uint16_t* data = reinterpret_cast<const uint16_t*>(dataPtr);
            for(size_t i = 0; i < accessor.count; i++) {
                output[i] = glm::uvec4(data[i * 4 + 0], data[i * 4 + 1], data[i * 4 + 2],
                                       data[i * 4 + 3]);
            }
        } else {
            Log::debug() << "Unsupported component type for JOINTS_0.";
        }
    } else if constexpr(std::is_same<T, glm::vec4>::value) {
        const glm::vec4* data = reinterpret_cast<const glm::vec4*>(dataPtr);
        std::copy(data, data + accessor.count, output.begin());
    }
}

void extractIndices(const tinygltf::Primitive& primitive, const tinygltf::Model& model,
                    std::vector<unsigned int>& output) {
    if(primitive.indices < 0) return;

    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    const unsigned char* dataPtr =
        buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

    switch(accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            const uint8_t* indices = reinterpret_cast<const uint8_t*>(dataPtr);
            output.assign(indices, indices + accessor.count);
            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            const uint16_t* indices = reinterpret_cast<const uint16_t*>(dataPtr);
            output.assign(indices, indices + accessor.count);
            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            const uint32_t* indices = reinterpret_cast<const uint32_t*>(dataPtr);
            output.assign(indices, indices + accessor.count);
            break;
        }
        default:
            Log::error() << "Unsupported index format.";
            break;
    }
}

} // namespace

GltfLoader::GltfLoader(const std::filesystem::path& path) : mPath(path) {
    Log::debug() << "Creating GltfLoader for '" << mPath.string() << "'.";
}

bool GltfLoader::load(ObjResource& resource) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret;
    if(mPath.extension() == ".obj") {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, mPath.string());
    } else {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, mPath.string());
    }
    if(!warn.empty()) {
        Log::warn() << "TinyGLTF warning: " << warn;
    }
    if(!ret) {
        Log::error() << "TinyGLTF error: " << err;
        return false;
    }

    if(model.animations.size() > 0) {
        resource.animationContainer = std::make_unique<AnimationContainer>(model);
    }
    loadMeshes(resource, model);
    loadMaterials(resource, model);
    return true;
}

void GltfLoader::loadMeshes(ObjResource& resource, const tinygltf::Model& model) {
    std::vector<ObjResource::Vertex> outVertices;

    // Traverse scene graph to apply transforms in the correct order
    std::stack<std::tuple<int, int>>
        nodeStack; // Stack to store {node index, parent skin}

    if(model.scenes.size() > 1) {
        Log::warn() << "GLTF file has multiple scenes. Only the first will be loaded.";
    }

    // Push all root nodes with identity transform
    for(int rootNodeIndex : model.scenes[0].nodes) {
        nodeStack.push({rootNodeIndex, model.nodes[rootNodeIndex].skin});
    }

    while(!nodeStack.empty()) {
        auto [nodeIndex, parentSkin] = nodeStack.top();
        int currentSkin = parentSkin;
        nodeStack.pop();

        const tinygltf::Node& node = model.nodes[nodeIndex];
        if(node.skin >= 0) {
            currentSkin = node.skin;
        }

        if(node.mesh >= 0) {
            if(resource.animationContainer) {
                AnimationContainer& animation = *resource.animationContainer;
                Log::debug() << "Loading mesh '" << model.meshes[node.mesh].name
                             << "' with animation.";
                auto objMesh =
                    loadMesh(resource, outVertices, model, model.meshes[node.mesh],
                             animation.getNode(nodeIndex));

                if(objMesh && currentSkin >= 0) {
                    Log::debug() << "Fetching skin " << currentSkin << " for mesh '"
                                 << objMesh->name << "'.";
                    objMesh->skin = animation.getSkin(currentSkin);
                    if(!objMesh->skin) {
                        Log::warn() << "Failed to find skin with index " << currentSkin
                                    << " for mesh '" << objMesh->name << "'.";
                    }
                }
            } else {
                Log::debug() << "Loading mesh '" << model.meshes[node.mesh].name << "'.";
                loadMesh(resource, outVertices, model, model.meshes[node.mesh]);
            }
        }

        for(int childIndex : node.children) {
            nodeStack.push(
                {childIndex, currentSkin}); // Push child node with inherited skin
        }
    }

    // Upload interleaved vertex data to GPU
    resource.vertexBuffer.setData(GL_ARRAY_BUFFER, outVertices);
}

// Returns the ID of the created mesh
ObjMesh::Ptr GltfLoader::loadMesh(ObjResource& resource,
                                  std::vector<ObjResource::Vertex>& outVertices,
                                  const tinygltf::Model& model,
                                  const tinygltf::Mesh& mesh, AnimationNode* node) {
    if(mesh.primitives.size() > 1) {
        Log::warn() << "Mesh has multiple primitives. Only the first will be loaded.";
    }

    const tinygltf::Primitive& primitive =
        mesh.primitives[0]; // Assume one primitive per mesh
    if(primitive.mode != TINYGLTF_MODE_TRIANGLES) {
        Log::warn() << "Skipping non-triangle primitive.";
        return {};
    }

    size_t baseIndex = outVertices.size(); // Offset for this mesh's indices

    // Extract vertex attributes
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::uvec4> joints; // New vector for joint indices
    std::vector<glm::vec4> weights; // New vector for joint weights

    extractAttribute(primitive, model, "POSITION", positions);
    extractAttribute(primitive, model, "NORMAL", normals);
    extractAttribute(primitive, model, "TEXCOORD_0", texcoords);
    extractAttribute(primitive, model, "JOINTS_0",
                     joints); // New extraction for joints
    extractAttribute(primitive, model, "WEIGHTS_0",
                     weights); // New extraction for weights

    if(normals.size() != positions.size() || texcoords.size() != positions.size()) {
        Log::warn() << "GLTF primitive has inconsistent attribute sizes.";
        return {};
    }

    // Get material ID (GLTF assigns materials per primitive)
    unsigned int materialId = (primitive.material >= 0) ? primitive.material : 0;

    // Ensure sizes match
    size_t vertexCount = positions.size();
    normals.resize(vertexCount, glm::vec3(0.0f));   // Default normal if missing
    texcoords.resize(vertexCount, glm::vec2(0.0f)); // Default texcoord if missing
    joints.resize(vertexCount, glm::uvec4(0));      // Default joints if missing
    weights.resize(vertexCount, glm::vec4(0.0f));   // Default weights if missing

    // Create interleaved vertex buffer
    outVertices.reserve(outVertices.size() + vertexCount);
    for(size_t i = 0; i < vertexCount; i++) {
        outVertices.emplace_back(positions[i], normals[i], texcoords[i], materialId,
                                 joints[i], weights[i]);
    }

    // Extract index buffer
    std::vector<unsigned int> primitiveIndices;
    extractIndices(primitive, model, primitiveIndices);

    // Adjust indices to match the current vertex offset
    for(unsigned int& index : primitiveIndices) {
        index += baseIndex;
    }

    // Store index buffer in ObjMesh
    auto objMesh = ObjMesh::create(resource, mesh.name, primitiveIndices, node);
    resource.objMeshes.emplace_back(objMesh);
    return objMesh;
}

void GltfLoader::loadMaterials(ObjResource& resource, const tinygltf::Model& model) {
    std::vector<ObjMaterial> objMaterials;
    for(const tinygltf::Material& gltfMaterial : model.materials) {
        ObjMaterial material = {};

        // Base Color (GLTF stores it as RGBA)
        if(gltfMaterial.pbrMetallicRoughness.baseColorFactor.size() == 4) {
            material.baseColor[0] = gltfMaterial.pbrMetallicRoughness.baseColorFactor[0];
            material.baseColor[1] = gltfMaterial.pbrMetallicRoughness.baseColorFactor[1];
            material.baseColor[2] = gltfMaterial.pbrMetallicRoughness.baseColorFactor[2];
            material.alpha = gltfMaterial.pbrMetallicRoughness.baseColorFactor[3];
        } else {
            material.baseColor[0] = 1.0f; // Default to white
            material.baseColor[1] = 1.0f;
            material.baseColor[2] = 1.0f;
            material.alpha = 1.0f;
        }

        // Emission Color
        if(gltfMaterial.emissiveFactor.size() == 3) {
            material.emission[0] = gltfMaterial.emissiveFactor[0];
            material.emission[1] = gltfMaterial.emissiveFactor[1];
            material.emission[2] = gltfMaterial.emissiveFactor[2];
        } else {
            material.emission[0] = material.emission[1] = material.emission[2] = 0.0f;
        }

        // Roughness & Metallic Factors
        material.roughness =
            static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
        material.metallic =
            static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
        material.sheen =
            0.0f; // Not supported in GLTF core, but could be added via extensions.

        // Texture Indices
        material.baseColorTextureIndex =
            gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        if(material.baseColorTextureIndex < 0) material.baseColorTextureIndex = -1;

        material.metallicRoughnessTextureIndex =
            gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
        if(material.metallicRoughnessTextureIndex < 0)
            material.metallicRoughnessTextureIndex = -1;

        material.normalTextureIndex = gltfMaterial.normalTexture.index;
        if(material.normalTextureIndex < 0) material.normalTextureIndex = -1;

        material.emissiveTextureIndex = gltfMaterial.emissiveTexture.index;
        if(material.emissiveTextureIndex < 0) material.emissiveTextureIndex = -1;

        objMaterials.push_back(material);
    }

    resource.materialUniformBuffer.setData(GL_UNIFORM_BUFFER, objMaterials);
}
