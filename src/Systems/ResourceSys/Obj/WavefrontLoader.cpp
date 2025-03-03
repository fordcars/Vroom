#include "WavefrontLoader.hpp"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <utility>

#include "Log.hpp"
#include "ObjMaterial.hpp"
#include "ObjResource.hpp"

WavefrontLoader::WavefrontLoader(const std::filesystem::path& path) : mPath(path) {
    Log::debug() << "Creating WavefrontLoader for '" << mPath.string() << "'.";
}

bool WavefrontLoader::load(ObjResource& resource) {
    // https://github.com/tinyobjloader/tinyobjloader?tab=readme-ov-file
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;

    if(!reader.ParseFromFile(mPath.string(), reader_config)) {
        if(!reader.Error().empty()) {
            Log::error() << "TinyObjReader error while loading '" << mPath.string()
                         << "': " << reader.Error();
            return false;
        }
    }

    if(!reader.Warning().empty()) {
        Log::warn() << "TinyObjReader warning while loading '" << mPath.string()
                    << "': " << reader.Warning();
    }

    loadOnGPU(resource, reader);
    return true;
}

void WavefrontLoader::loadOnGPU(ObjResource& resource, const tinyobj::ObjReader& reader) {
    loadMeshes(resource, reader);

    // Convert OBJ materials to PBR-compatible format
    std::vector<ObjMaterial> objMaterials;
    for(const tinyobj::material_t& objMat : reader.GetMaterials()) {
        ObjMaterial mat = {};

        // Convert diffuse to baseColor
        mat.baseColor =
            glm::vec3(objMat.diffuse[0], objMat.diffuse[1], objMat.diffuse[2]);
        mat.alpha = 1.0f - objMat.dissolve; // OBJ uses "dissolve" for transparency

        // Convert specular to roughness/metallic approximation
        mat.metallic = glm::length(
            glm::vec3(objMat.specular[0], objMat.specular[1], objMat.specular[2]));
        mat.roughness = 1.0f - objMat.shininess / 1000.0f; // Roughness approximation

        // Emission (same in both OBJ & PBR)
        mat.emission =
            glm::vec3(objMat.emission[0], objMat.emission[1], objMat.emission[2]);

        // Set default texture indices (no textures in OBJ by default)
        mat.baseColorTextureIndex = -1;
        mat.metallicRoughnessTextureIndex = -1;
        mat.normalTextureIndex = -1;
        mat.emissiveTextureIndex = -1;

        objMaterials.push_back(mat);
    }

    // Upload materials to GPU
    resource.materialUniformBuffer.setData(GL_UNIFORM_BUFFER, objMaterials);
    Log::debug() << "Loaded " << objMaterials.size() << " materials.";
}

void WavefrontLoader::loadMeshes(ObjResource& resource,
                                 const tinyobj::ObjReader& reader) {
    const unsigned VERTICES_PER_FACE = 3;
    const glm::vec3 UNINITIALIZED_VEC3(-1.0f);
    const glm::vec2 UNINITIALIZED_VEC2(-1.0f);

    // We are given per-vertex attributes (position, normal, texcoords, colors),
    // and per-FACE material id (sad!). This does not work well with OpenGL
    // index buffers. To fix this, we make the material id per-VERTEX. To do this,
    // we find any vertex which is used by faces with different material ids, and
    // duplicate it. This ensures every face is composed of 3 vertices with the
    // same material id, greatly simplifying the rendering process.

    // Copy all attributes :(
    // NOTE: We convert all attributes to only use a single index buffer (vertex index).
    // -1 to identify uninitialized values.
    const auto& attribs = reader.GetAttrib();
    std::vector<ObjResource::Vertex> outVertices(
        attribs.vertices.size() / 3,
        ObjResource::Vertex{UNINITIALIZED_VEC3, UNINITIALIZED_VEC3, UNINITIALIZED_VEC2,
                            0});

    Log::debug() << "Found " << attribs.vertices.size() << " vertex positions, "
                 << attribs.normals.size() << " normals and " << attribs.texcoords.size()
                 << " texcoords.";

    for(const auto& shape : reader.GetShapes()) {
        std::vector<unsigned int> meshVertexIndices;
        meshVertexIndices.reserve(shape.mesh.indices.size());
        size_t originalVertexCount = attribs.vertices.size() / 3; // For logs
        Log::debug() << "Loading mesh '" << shape.name << "' with "
                     << shape.mesh.indices.size() / VERTICES_PER_FACE << " faces.";

        for(size_t indexI = 0; indexI < shape.mesh.indices.size(); ++indexI) {
            int vertexI = shape.mesh.indices[indexI].vertex_index;
            int normalI = shape.mesh.indices[indexI].normal_index;
            int texcoordI = shape.mesh.indices[indexI].texcoord_index;

            glm::vec3 newVertex(attribs.vertices[vertexI * 3],
                                attribs.vertices[vertexI * 3 + 1],
                                attribs.vertices[vertexI * 3 + 2]);
            glm::vec3 newNormal = normalI >= 0
                                      ? glm::vec3(attribs.normals[normalI * 3],
                                                  attribs.normals[normalI * 3 + 1],
                                                  attribs.normals[normalI * 3 + 2])
                                      : glm::vec3(0.0f);
            glm::vec2 newTexcoord = texcoordI >= 0
                                        ? glm::vec2(attribs.texcoords[texcoordI * 2],
                                                    attribs.texcoords[texcoordI * 2 + 1])
                                        : glm::vec2(0.0f);
            GLuint newMaterialId = shape.mesh.material_ids[indexI / VERTICES_PER_FACE];

            if((outVertices[vertexI].normal != UNINITIALIZED_VEC3 &&
                outVertices[vertexI].normal != newNormal) ||
               (outVertices[vertexI].texcoord != UNINITIALIZED_VEC2 &&
                outVertices[vertexI].texcoord != newTexcoord) ||
               (outVertices[vertexI].materialId != -1 &&
                outVertices[vertexI].materialId != newMaterialId)) {
                // Bummer! A previous vertex set the attributes of this vertex.
                // We must duplicate it.
                outVertices.push_back(outVertices[vertexI]);
            }

            // Move attributes
            outVertices[vertexI] = {newVertex, newNormal, newTexcoord, newMaterialId};

            // Copy our vertex index
            meshVertexIndices.emplace_back(vertexI);
        }

        // Add mesh
        resource.objMeshes.emplace_back(
            ObjMesh::create(resource, shape.name, meshVertexIndices));
        size_t duplicatedVertices = meshVertexIndices.size() - originalVertexCount;
        Log::debug() << "Duplicated " << outVertices.size() << " vertices for mesh '"
                     << shape.name << "' (originally had " << originalVertexCount
                     << " vertices).";
    }

    // Load interleaved attributes
    resource.vertexBuffer.setData(GL_ARRAY_BUFFER, outVertices);
}
