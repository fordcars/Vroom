#include "ObjResource.hpp"

#include <vector>
#include <tiny_obj_loader.h>
#include "Log.hpp"

ObjResource::ObjResource(const std::string& name, const std::filesystem::path& path)
    : Resource(name)
    , mPath(path) {
    load();
}

bool ObjResource::load() {
    // https://github.com/tinyobjloader/tinyobjloader?tab=readme-ov-file
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

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

    return true;
}