#include "ObjResource.hpp"

#include "Log.hpp"

ObjResource::ObjResource(const std::string& name, const std::filesystem::path& path)
    : Resource(name)
    , mPath(path) {
    load();
}

const tinyobj::attrib_t& ObjResource::getAttrib() const {
    return mReader.GetAttrib();
}

const std::vector<tinyobj::shape_t>& ObjResource::getShapes() const {
    return mReader.GetShapes();
}

const std::vector<tinyobj::material_t>& ObjResource::getMaterials() const {
    return mReader.GetMaterials();
}

bool ObjResource::load() {
    // https://github.com/tinyobjloader/tinyobjloader?tab=readme-ov-file
    tinyobj::ObjReaderConfig reader_config;

    if (!mReader.ParseFromFile(mPath.string(), reader_config)) {
        if (!mReader.Error().empty()) {
            Log::error() << "TinyObjReader error while loading '"
                << mPath.string() << "': " << mReader.Error();
            return false;
        }
    }

    if (!mReader.Warning().empty()) {
        Log::warn() << "TinyObjReader warning while loading '"
            << mPath.string() << "': " << mReader.Warning();
    }

    return true;
}