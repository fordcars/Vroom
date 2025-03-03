#include "GltfLoader.hpp"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <utility>

#include "Log.hpp"
#include "ObjAnimation.hpp"
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

    loadMeshes(resource, model);
    loadMaterials(resource, model);

    return true;
}

void GltfLoader::loadMeshes(ObjResource& resource, const tinygltf::Model& model) {
    std::vector<ObjResource::Vertex> interleavedVertices;
    std::vector<unsigned int> indices;

    for(const tinygltf::Mesh& mesh : model.meshes) {
        for(const tinygltf::Primitive& primitive : mesh.primitives) {
            if(primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                Log::warn() << "Skipping non-triangle primitive.";
                continue;
            }

            // Extract vertex attributes
            std::vector<glm::vec3> positions;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> texcoords;

            extractAttribute(primitive, model, "POSITION", positions);
            extractAttribute(primitive, model, "NORMAL", normals);
            extractAttribute(primitive, model, "TEXCOORD_0", texcoords);

            if(normals.size() != positions.size() ||
               texcoords.size() != positions.size()) {
                Log::error() << "GLTF primitive has inconsistent attribute sizes.";
                continue;
            }

            // Get material ID (GLTF assigns materials per primitive)
            unsigned int materialId = (primitive.material >= 0) ? primitive.material : 0;

            // Ensure sizes match
            size_t vertexCount = positions.size();
            normals.resize(vertexCount, glm::vec3(0.0f));   // Default normal if missing
            texcoords.resize(vertexCount, glm::vec2(0.0f)); // Default texcoord if missing

            // Create interleaved vertex buffer
            size_t baseIndex = interleavedVertices.size();
            interleavedVertices.reserve(interleavedVertices.size() + vertexCount);
            for(size_t i = 0; i < vertexCount; i++) {
                interleavedVertices.emplace_back(positions[i], normals[i], texcoords[i],
                                                 materialId);
            }

            // Extract index buffer
            std::vector<unsigned int> primitiveIndices;
            extractIndices(primitive, model, primitiveIndices);

            // Adjust indices to match interleavedVertices
            for(unsigned int& index : primitiveIndices) {
                index += baseIndex;
            }
            indices.reserve(indices.size() + primitiveIndices.size());
            indices.insert(indices.end(), primitiveIndices.begin(),
                           primitiveIndices.end());

            // Store index buffer in ObjMesh
            resource.objMeshes.push_back(
                ObjMesh::create(resource, mesh.name, primitiveIndices));
        }
    }

    // Upload interleaved vertex data to GPU
    resource.vertexBuffer.setData(GL_ARRAY_BUFFER, interleavedVertices);
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