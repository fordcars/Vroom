#include "ObjResource.hpp"
#include <utility>
#include <glad/glad.h>

#include "Log.hpp"
#include "ObjMaterial.hpp"

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
    // Load attributes
    const auto& attribs = reader.GetAttrib();
    vertexBuffer.setData(GL_ARRAY_BUFFER, attribs.vertices);
    normalBuffer.setData(GL_ARRAY_BUFFER, attribs.normals);
    texCoordBuffer.setData(GL_ARRAY_BUFFER, attribs.texcoords);
    colorBuffer.setData(GL_ARRAY_BUFFER, attribs.colors);

    // Load shapes
    const auto& shapes = reader.GetShapes();
    for(const auto& shape : shapes) {
        objMeshes.emplace_back(ObjMesh::create(*this, shape));
    }

    // Load materials in uniform buffer object
    std::vector<ObjMaterial> objMaterials(
        reader.GetMaterials().begin(),
        reader.GetMaterials().end()
    );
    materialUniformBuffer.setData(GL_UNIFORM_BUFFER, objMaterials);
}
