#include "ResourceSys.hpp"

#include <string>
#include <memory>
#include "Log.hpp"
#include "Constants.hpp"

// Static
ResourceSys& ResourceSys::get() {
    static std::unique_ptr<ResourceSys> instance = std::make_unique<ResourceSys>();
    return *instance;
}

// Will load all resources in all subdirs of the resource dir.
// Resources will have the name of the file (without extension).
bool ResourceSys::loadResources() {
    Log::info() << "Loading resources from "
        << Constants::RESOURCE_DIR << "/ directory...";
    return loadResourcesFromDir(Constants::RESOURCE_DIR);
}

ObjResource::CPtr ResourceSys::getObjResource(const std::string& name) const {
    if(mObjResources.find(name) == mObjResources.end()) {
        Log::error() << "Cannot find obj resource '" << name << "'!";
        throw std::invalid_argument("No obj resource with name '" + name + "'");
    }

    return mObjResources.at(name);
}

bool ResourceSys::loadResourcesFromDir(const std::filesystem::path& dirPath) {
    namespace fs = std::filesystem;
    bool success = true;

    if(!fs::is_directory(dirPath)) return false;
    for(const auto& entry : fs::directory_iterator(dirPath)) {
        if(fs::is_directory(entry.path())) {
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

    bool alreadyExists = false;
    if(type == ".obj") {
        if(mObjResources.find(name) != mObjResources.end()) alreadyExists = true;
        else mObjResources.insert({name, ObjResource::create(path)});
    } else if(type == ".glsl") {
        if(mShaderResources.find(name) != mShaderResources.end()) alreadyExists = true;
        else mShaderResources.insert({name, ShaderResource::create(path)});
        /// TODO: deal with separate v and f shader source files
    }

    if(alreadyExists) {
        Log::error() << "Cannot load " << type << " resource '" << path.string()
            << "', a " << type << " resource with the name '" << name
            << "' already exists!";
        return false;
    }
    return true;
}
