#include "ObjResource.hpp"
#include <utility>
#include <glad/glad.h>

#include "Log.hpp"

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
    vertexBuffer.setData(attribs.vertices);
    normalBuffer.setData(attribs.normals);
    texCoordBuffer.setData(attribs.texcoords);
    colorBuffer.setData(attribs.colors);

    // Load shapes
    const auto& shapes = reader.GetShapes();
    for(const auto& shape : shapes) {
        objMeshes.emplace_back(ObjMesh::create(*this, shape));
    }
}
