#include "GltfLoader.hpp"

#include <glad/glad.h>
#include <stb_image.h> // Used by TinyGLTF

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
        warn.pop_back(); // Remove trailing newline
        Log::warn() << "TinyGLTF warning: " << warn;
    }
    if(!ret && !err.empty()) {
        err.pop_back(); // Remove trailing newline
        Log::error() << "TinyGLTF error: " << err;
        if(stbi_failure_reason()) {
            Log::error() << "STB Image error: " << stbi_failure_reason();
        }
        return false;
    }

    if(model.animations.size() > 0) {
        resource.animationContainer = std::make_unique<AnimationContainer>(model);
    }

    loadImages(resource, model);
    loadTextures(resource, model);
    loadMaterials(resource, model);
    loadMeshes(resource, model);
    return true;
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

        objMaterials.push_back(material);
    }

    resource.materialUniformBuffer.setData(GL_UNIFORM_BUFFER, objMaterials);
}

void GltfLoader::loadImages(ObjResource& resource, const tinygltf::Model& model) {
    for(const tinygltf::Image& gltfImage : model.images) {
        if(gltfImage.image.empty()) {
            Log::warn() << "Empty image data for image '" << gltfImage.name << "'.";
            resource.objImages.emplace_back(
                ObjImage::create(gltfImage.name, 0, 0, 0)); // Empty image
            continue;
        }

        // Generate GL texture
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);

        // Load image data
        Log::debug() << "Loading image '" << gltfImage.name << "' with dimensions "
                     << gltfImage.width << "x" << gltfImage.height << " and "
                     << gltfImage.component << " components.";

        GLenum format = (gltfImage.component == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, gltfImage.width, gltfImage.height, 0,
                     format, GL_UNSIGNED_BYTE, gltfImage.image.data());

        glGenerateMipmap(GL_TEXTURE_2D);

        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            Log::error() << "OpenGL error: " << error;
        }

        // Create ObjImage
        auto objImage =
            ObjImage::create(gltfImage.name, texId, gltfImage.width, gltfImage.height);
        resource.objImages.emplace_back(objImage);
    }
}

void GltfLoader::loadTextures(ObjResource& resource, const tinygltf::Model& model) {
    auto emptyImage = ObjImage::create("empty_image", 0, 0, 0);

    for(const tinygltf::Texture& gltfTexture : model.textures) {
        if(gltfTexture.source < 0) {
            Log::warn() << "Texture '" << gltfTexture.name << "' has no image source.";
            resource.objTextures.emplace_back(
                ObjTexture::create(gltfTexture.name, emptyImage, 0)); // Empty texture
            continue;
        }

        if(gltfTexture.source >= static_cast<int>(resource.objImages.size())) {
            Log::warn() << "Texture '" << gltfTexture.name
                        << "' has an invalid image source.";
            resource.objTextures.emplace_back(
                ObjTexture::create(gltfTexture.name, emptyImage, 0)); // Empty texture
            continue;
        }

        const tinygltf::Sampler& sampler = model.samplers[gltfTexture.sampler];

        // Create GL sampler
        GLuint samplerId;
        glGenSamplers(1, &samplerId);

        GLenum minFilter =
            sampler.minFilter != -1 ? sampler.minFilter : GL_LINEAR_MIPMAP_LINEAR;
        GLenum magFilter = sampler.magFilter != -1 ? sampler.magFilter : GL_LINEAR;
        GLenum wrapS = sampler.wrapS != -1 ? sampler.wrapS : GL_REPEAT;
        GLenum wrapT = sampler.wrapT != -1 ? sampler.wrapT : GL_REPEAT;

        glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, minFilter);
        glSamplerParameteri(samplerId, GL_TEXTURE_MAG_FILTER, magFilter);
        glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_S, wrapS);
        glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_T, wrapT);

        // Create ObjTexture
        auto objTexture = ObjTexture::create(
            gltfTexture.name, resource.objImages[gltfTexture.source], samplerId);
        resource.objTextures.emplace_back(objTexture);
    }
}

void GltfLoader::loadMeshes(ObjResource& resource, const tinygltf::Model& model) {
    struct StackEntry {
        int nodeIndex = 0;
        int skin = -1;
        glm::mat4 transform = glm::mat4(1.0f);
    };

    std::vector<ObjResource::Vertex> outVertices;

    // Traverse scene graph to apply transforms in the correct order
    std::stack<StackEntry> nodeStack;

    if(model.scenes.size() > 1) {
        Log::warn() << "GLTF file has multiple scenes. Only the first will be loaded.";
    }

    // Push all root nodes with identity transform
    for(int rootNodeIndex : model.scenes[0].nodes) {
        nodeStack.push({rootNodeIndex, model.nodes[rootNodeIndex].skin});
    }

    while(!nodeStack.empty()) {
        int nodeIndex = nodeStack.top().nodeIndex;
        int currentSkin = nodeStack.top().skin;
        glm::mat4 transform = nodeStack.top().transform;
        nodeStack.pop();

        const tinygltf::Node& node = model.nodes[nodeIndex];
        if(node.skin >= 0) {
            currentSkin = node.skin;
        }

        // Apply node transform
        glm::vec3 translation = node.translation.empty()
                                    ? glm::vec3(0.0f)
                                    : glm::vec3(node.translation[0], node.translation[1],
                                                node.translation[2]);
        glm::quat rotation = node.rotation.empty()
                                 ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
                                 : glm::quat(node.rotation[3], node.rotation[0],
                                             node.rotation[1], node.rotation[2]);
        glm::vec3 scale = node.scale.empty()
                              ? glm::vec3(1.0f)
                              : glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

        glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), translation) *
                                   glm::mat4_cast(rotation) *
                                   glm::scale(glm::mat4(1.0f), scale);

        transform = transform * localTransform;

        if(node.mesh >= 0) {
            loadPrimitives(resource, outVertices, model, nodeIndex, currentSkin, transform);
        }

        for(int childIndex : node.children) {
            nodeStack.push({childIndex, currentSkin,
                            transform}); // Push child node with inherited skin
        }
    }

    // Upload interleaved vertex data to GPU
    resource.vertexBuffer.setData(GL_ARRAY_BUFFER, outVertices);
    resource.vertices = std::move(outVertices);
}

// Each glTF primitive will generate an ObjMesh
void GltfLoader::loadPrimitives(ObjResource& resource,
                                std::vector<ObjResource::Vertex>& outVertices,
                                const tinygltf::Model& model, int gltfNodeIndex,
                                int gltfSkinIndex, const glm::mat4& meshTransform) {
    const tinygltf::Node& node = model.nodes[gltfNodeIndex];
    if(node.mesh < 0) {
        Log::warn() << "Node " << gltfNodeIndex << " has no mesh.";
        return;
    }
    const tinygltf::Mesh& mesh = model.meshes[node.mesh];

    // Get animation node if mesh is animated
    AnimationNode* animationNode = nullptr;
    if(resource.animationContainer) {
        animationNode = resource.animationContainer->getNode(gltfNodeIndex);
        Log::debug() << "Loading animated mesh '" << mesh.name << "' with "
                     << mesh.primitives.size() << " primitive(s).";
    } else {
        Log::debug() << "Loading mesh '" << mesh.name << "' with "
                     << mesh.primitives.size() << " primitive(s).";
    }

    // Get skin if present
    Skin::Ptr skin = nullptr;
    if(resource.animationContainer && gltfSkinIndex >= 0) {
        Log::debug() << "Fetching skin " << gltfSkinIndex << " for mesh '" << mesh.name
                     << "'.";
        skin = resource.animationContainer->getSkin(gltfSkinIndex);
        if(!skin) {
            Log::warn() << "Failed to find skin with index " << gltfSkinIndex
                        << " for mesh '" << mesh.name << "'.";
        }
    }

    for(size_t i = 0; i < mesh.primitives.size(); i++) {
        const tinygltf::Primitive& primitive = mesh.primitives[i];

        if(primitive.mode != TINYGLTF_MODE_TRIANGLES) {
            Log::warn() << "Skipping non-triangle primitive.";
            continue;
        }

        size_t baseIndex = outVertices.size(); // Offset for this mesh's indices

        // Extract vertex attributes
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texcoords;
        std::vector<glm::uvec4> joints;
        std::vector<glm::vec4> weights;

        extractAttribute(primitive, model, "POSITION", positions);
        extractAttribute(primitive, model, "NORMAL", normals);
        extractAttribute(primitive, model, "TEXCOORD_0", texcoords);
        extractAttribute(primitive, model, "JOINTS_0", joints);
        extractAttribute(primitive, model, "WEIGHTS_0", weights);

        if(normals.size() != positions.size() || texcoords.size() != positions.size()) {
            Log::warn() << "GLTF primitive has inconsistent attribute sizes.";
            return;
        }

        // Get material ID (GLTF assigns materials per primitive)
        unsigned int materialId = (primitive.material >= 0) ? primitive.material : 0;

        // Resize vectors to make sure they have the same size
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
        auto objMesh = ObjMesh::create(resource, mesh.name + "_p" + std::to_string(i),
                                       std::move(primitiveIndices));
        objMesh->transform = meshTransform;
        objMesh->skin = skin;
        objMesh->animationNode = animationNode;

        resource.objMeshes.emplace_back(objMesh);
        setMeshTextures(resource, objMesh, model, primitive);
    }
}

void GltfLoader::setMeshTextures(ObjResource& resource, ObjMesh::Ptr mesh,
                                 const tinygltf::Model& model,
                                 const tinygltf::Primitive& primitive) {
    auto getTexture = [&resource](const tinygltf::TextureInfo& texInfo,
                                  const std::string& type) -> ObjTexture::Ptr {
        if(texInfo.texCoord > 0) {
            Log::warn() << "Texture coordinate sets other than 0 are not supported.";
        }

        if(texInfo.index < 0) return nullptr;
        Log::debug() << "Using image " << resource.objTextures[texInfo.index]->image->name
                     << " for mesh " << type << " texture.";
        return resource.objTextures[texInfo.index];
    };

    auto getNormalTexture =
        [&resource](const tinygltf::NormalTextureInfo& texInfo) -> ObjTexture::Ptr {
        if(texInfo.texCoord > 0) {
            Log::warn()
                << "Normal texture coordinate sets other than 0 are not supported.";
        }

        if(texInfo.index < 0) return nullptr;
        Log::debug() << "Using image " << resource.objTextures[texInfo.index]->image->name
                     << " for mesh normal map texture.";
        return resource.objTextures[texInfo.index];
    };

    if(primitive.material < 0) {
        Log::debug() << "Primitive has no material.";
        return;
    }

    const tinygltf::Material& gltfMaterial = model.materials[primitive.material];
    mesh->baseColorTexture =
        getTexture(gltfMaterial.pbrMetallicRoughness.baseColorTexture, "base color");
    mesh->normalTexture = getNormalTexture(gltfMaterial.normalTexture);
    mesh->metallicRoughnessTexture = getTexture(
        gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture, "metallic roughness");
    mesh->emissiveTexture = getTexture(gltfMaterial.emissiveTexture, "emissive");
    mesh->normalScale = static_cast<float>(gltfMaterial.normalTexture.scale);
}