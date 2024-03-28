#include "ObjResource.hpp"
#include <utility>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Log.hpp"
#include "ObjMaterial.hpp"

namespace {
    // Returns new vertex index
    int duplicateVertex(
        std::vector<glm::vec3>& vertices,
        std::vector<glm::vec3>& normals,
        std::vector<glm::vec2>& texcoords,
        std::vector<GLint>& materialIds,
        int vertexI) {
            vertices.emplace_back(vertices[vertexI]);
            normals.emplace_back(normals[vertexI]);
            texcoords.emplace_back(texcoords[vertexI]);
            materialIds.emplace_back(materialIds[vertexI]);

            return static_cast<int>(vertices.size() - 1);
        }
}

ObjResource::ObjResource(const std::filesystem::path& path)
    : mPath(path) {
    load();
}

bool ObjResource::load() {
    // https://github.com/tinyobjloader/tinyobjloader?tab=readme-ov-file
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;

    if (!reader.ParseFromFile(mPath.string(), reader_config)) {
        if (!reader.Error().empty()) {
            Log::error() << "TinyObjReader error while loading '"
                << mPath.string() << "': " << reader.Error();
            return false;
        }
    }

    if (!reader.Warning().empty()) {
        Log::warn() << "TinyObjReader warning while loading '"
            << mPath.string() << "': " << reader.Warning();
    }

    loadOnGPU(reader);
    return true;
}

void ObjResource::loadOnGPU(const tinyobj::ObjReader& reader) {
    loadMeshes(reader);

    // Load materials in uniform buffer
    std::vector<ObjMaterial> objMaterials(
        reader.GetMaterials().begin(),
        reader.GetMaterials().end()
    );
    materialUniformBuffer.setData(GL_UNIFORM_BUFFER, objMaterials);

    Log::debug() << "Loaded " << objMaterials.size() << " materials.";
}

void ObjResource::loadMeshes(const tinyobj::ObjReader& reader) {
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
    // Convert all attributes to only use a single index buffer (vertex index).
    // -1 to identify uninitialized values.
    const auto& attribs = reader.GetAttrib();
    std::vector<glm::vec3> vertices(attribs.vertices.size()/3, UNINITIALIZED_VEC3);
    std::vector<glm::vec3> normals(attribs.vertices.size()/3, UNINITIALIZED_VEC3);
    std::vector<glm::vec2> texcoords(attribs.vertices.size()/3, UNINITIALIZED_VEC2);
    std::vector<GLint>   materialIds(attribs.vertices.size()/3, -1);

    Log::debug() << "Found " << attribs.vertices.size() << " vertex positions, "
        << attribs.normals.size() << " normals and " << attribs.texcoords.size()
        << " texcoords.";

    for(const auto& shape : reader.GetShapes()) {
        std::vector<unsigned int> meshVertexIndices;
        meshVertexIndices.reserve(shape.mesh.indices.size());
        size_t originalVertexCount = vertices.size()/3; // For logs

        for(size_t indexI = 0; indexI < shape.mesh.indices.size(); ++indexI) {
            int vertexI = shape.mesh.indices[indexI].vertex_index;
            int normalI = shape.mesh.indices[indexI].normal_index;
            int texcoordI = shape.mesh.indices[indexI].texcoord_index;

            glm::vec3 newVertex(
                attribs.vertices[vertexI * 3],
                attribs.vertices[vertexI * 3 + 1],
                attribs.vertices[vertexI * 3 + 2]);
            glm::vec3 newNormal = normalI >= 0 ? glm::vec3(
                attribs.normals[normalI * 3],
                attribs.normals[normalI * 3 + 1],
                attribs.normals[normalI * 3 + 2])
                : glm::vec3(0.0f);
            glm::vec2 newTexcoord = texcoordI >= 0 ? glm::vec2(
                attribs.texcoords[texcoordI * 2],
                attribs.texcoords[texcoordI * 2 + 1])
                : glm::vec2(0.0f);
            GLint newMaterialId = shape.mesh.material_ids[indexI / VERTICES_PER_FACE];

            if(
                   (normals[vertexI]     != UNINITIALIZED_VEC3 && normals[vertexI] != newNormal)
                || (texcoords[vertexI]   != UNINITIALIZED_VEC2 && texcoords[vertexI] != newTexcoord)
                || (materialIds[vertexI] != -1 && materialIds[vertexI] != newMaterialId)) {
                // Bummer! A previous vertex set the attributes of this vertex.
                // We must duplicate it.
                vertexI = duplicateVertex(vertices, normals, texcoords, materialIds, vertexI);
            }

            // Move attributes
            vertices[vertexI]    = std::move(newVertex);
            normals[vertexI]     = std::move(newNormal);
            texcoords[vertexI]   = std::move(newTexcoord);
            materialIds[vertexI] = std::move(newMaterialId);

            // Copy our vertex index
            meshVertexIndices.emplace_back(vertexI);
        }

        // Add mesh
        objMeshes.emplace_back(ObjMesh::create(*this, shape.name, meshVertexIndices));
        size_t duplicatedVertices = vertices.size()/3 - originalVertexCount;
        Log::debug() << "Duplicated " << duplicatedVertices << " vertices for mesh '"
            << shape.name << "' (originally had " << originalVertexCount << " vertices).";
    }

    // Load attributes
    vertexBuffer.setData(GL_ARRAY_BUFFER, vertices);
    normalBuffer.setData(GL_ARRAY_BUFFER, normals);
    texcoordBuffer.setData(GL_ARRAY_BUFFER, texcoords);
    materialIdBuffer.setData(GL_ARRAY_BUFFER, materialIds);
}
