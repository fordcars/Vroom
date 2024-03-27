#include "ObjResource.hpp"
#include <utility>
#include <glad/glad.h>

#include "Log.hpp"
#include "ObjMaterial.hpp"

// We currently only support vertex indices!
// We don't support normal or texcoord indices.
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
    // We are given per-vertex attributes (position, normal, texcoords, colors),
    // and per-FACE material id (sad!). This does not work well with OpenGL
    // index buffers. To fix this, we make the material id per-VERTEX. To do this,
    // we find any vertex which is used by faces with different material ids, and
    // duplicate it. This ensures every face is composed of 3 vertices with the
    // same material id, greatly simplifying the rendering process.

    // Copy all attributes :(
    // This is necessary, since we may add new vertices.
    const auto& attribs = reader.GetAttrib();
    auto vertices = attribs.vertices;
    auto normals = attribs.normals;
    auto texcoords = attribs.texcoords;
    auto colors = attribs.colors;

    // -2 to identify uninitialized values, since tinyobjloader uses -1.
    std::vector<GLint> materialIds(attribs.vertices.size()/3, -2);

    // Make sure all attributes have the same size
    if(normals.size() != vertices.size())
        normals = decltype(normals)(attribs.vertices.size(), 0.0f);
    if(texcoords.size() != vertices.size())
        texcoords = decltype(texcoords)((attribs.vertices.size()/3) * 2, 0.0f);
    if(colors.size() != vertices.size())
        colors = decltype(colors)(attribs.vertices.size(), 0.0f);

    for(const auto& shape : reader.GetShapes()) {
        std::vector<unsigned int> meshVertexIndices;
        meshVertexIndices.reserve(shape.mesh.indices.size());
        size_t originalVertexCount = vertices.size()/3; // For logs

        for(size_t indexI = 0; indexI < shape.mesh.indices.size(); ++indexI) {
            int vertexI = shape.mesh.indices[indexI].vertex_index;
            if(materialIds[vertexI] == -2) {
                // No material set yet! Lets use ours.
                materialIds[vertexI] = shape.mesh.material_ids[indexI / 3]; // 3 vertices per face
                meshVertexIndices.emplace_back(vertexI); // Copy vertex index
            } else {
                // Bummer! A previous vertex set this material. This means this
                // vertex is used with 2 different materials. We must duplicate this vertex.
                // Note: it is possible we duplicate the same vertex multiple times, for ex:
                // this vertex is used by many different faces with the same "new" material id.
                vertices.insert(vertices.end(),
                    vertices.begin() + vertexI * 3, vertices.begin() + vertexI * 3 + 3);
                normals.insert(normals.end(),
                    normals.begin() + vertexI * 3, normals.begin() + vertexI * 3 + 3);
                texcoords.insert(texcoords.end(),
                    texcoords.begin() + vertexI * 2, texcoords.begin() + vertexI * 2 + 2);
                colors.insert(colors.end(),
                    colors.begin() + vertexI * 3, colors.begin() + vertexI * 3 + 3);
                materialIds.emplace_back(shape.mesh.material_ids[indexI / 3]);

                // Our vertex index will point to our new, duplicated vertex.
                meshVertexIndices.push_back(static_cast<unsigned int>((vertices.size() / 3) - 1));
            }
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
    texCoordBuffer.setData(GL_ARRAY_BUFFER, texcoords);
    colorBuffer.setData(GL_ARRAY_BUFFER, colors);
    materialIdBuffer.setData(GL_ARRAY_BUFFER, materialIds);
}
