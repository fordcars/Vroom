#include "ResourceSys.hpp"

#include <string>
#include "Log.hpp"
#include "Constants.hpp"

// Will load all resources in all subdirs of the resource dir.
// Resources will have the name of the file (without extension).
bool ResourceSys::loadResources() {
    Log::info() << "Loading resources from "
        << Constants::RESOURCE_DIR << "/ directory...";
    return loadResourcesFromDir(Constants::RESOURCE_DIR);
}

bool ResourceSys::loadResourcesFromDir(const std::filesystem::path& dirPath) {
    namespace fs = std::filesystem;
    bool success = true;

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (fs::is_directory(entry.path())) {
            success &= loadResourcesFromDir(entry.path());
        } else {
            success &= loadResource(entry.path());
        }
    }

    return success;
}

bool ResourceSys::loadResource(const std::filesystem::path& path) {
    std::string name = path.stem().string();
    std::string type = path.extension().string();

    Log::debug() << "Loading resource '" << name
        << "' with type '" << type << "' from " << path.string();

    if(type == ".obj") {
        if(mObjResources.find(name) != mObjResources.end()) {
            Log::error() << "Cannot load obj resource '" << path.string()
                << "', an obj resource with the name '" << name
                << "' already exists!";
        }
        mObjResources.insert({name, ObjResource(name, path)});
    }
    return true;
}