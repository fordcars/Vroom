#include "ResourceSys.hpp"

#include <memory>
#include <string>

#include "Constants.hpp"
#include "Log.hpp"
#include "Obj/GltfLoader.hpp"
#include "Obj/WavefrontLoader.hpp"

// Static
ResourceSys& ResourceSys::get() {
    static std::unique_ptr<ResourceSys> instance = std::make_unique<ResourceSys>();
    return *instance;
}

// Will load all resources in all subdirs of the resource dir.
// Resources will have the name of the file (without extension).
bool ResourceSys::loadResources() {
    Log::info() << "Loading resources from " << Constants::RESOURCE_DIR
                << "/ directory...";
    bool res = loadResourcesFromDir(Constants::RESOURCE_DIR);
    Log::info() << "Done loading resources!";

    return res;
}

ObjResource::Ptr ResourceSys::getObjResource(const std::string& name) {
    if(mObjResources.find(name) == mObjResources.end()) {
        Log::error() << "Cannot find obj resource '" << name << "'!";
        throw std::invalid_argument("No obj resource with name '" + name + "'");
    }

    return mObjResources.at(name);
}

ShaderResource::CPtr ResourceSys::getShaderResource(const std::string& name) const {
    if(mShaderResources.find(name) == mShaderResources.end()) {
        Log::error() << "Cannot find shader resource '" << name << "'!";
        throw std::invalid_argument("No shader resource with name '" + name + "'");
    }

    return mShaderResources.at(name);
}

AudioResource::Ptr ResourceSys::getAudioResource(const std::string& name) {
    if(mAudioResources.find(name) == mAudioResources.end()) {
        Log::error() << "Cannot find audio resource '" << name << "'!";
        throw std::invalid_argument("No audio resource with name '" + name + "'");
    }

    return mAudioResources.at(name);
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

    // If a dot is present in the name, only keep whatever is before it
    size_t firstDot = name.find('.');
    if(firstDot != std::string::npos) name = name.substr(0, firstDot);

    Log::debug() << "Found resource '" << name << "' with type '" << type << "' from "
                 << path.string();

    bool alreadyExists = false;
    std::string resourceType;
    if(type == ".obj" || type == ".gltf" || type == ".glb") {
        if(mObjResources.find(name) != mObjResources.end()) {
            alreadyExists = true;
            resourceType = "object";
        } else {
            if(type == ".obj") {
                mObjResources.insert(
                    {name, ObjResource::create(std::make_unique<WavefrontLoader>(path))});
            } else {
                mObjResources.insert(
                    {name, ObjResource::create(std::make_unique<GltfLoader>(path))});
            }
        }
    } else if(type == ".wav" || type == ".flac" || type == ".mp3") {
        if(mAudioResources.find(name) != mAudioResources.end()) {
            alreadyExists = true;
            resourceType = "audio";
        } else {
            mAudioResources.insert({name, AudioResource::create(path)});
        }
    } else if(type == ".glsl") {
        if(mShaderResources.find(name) == mShaderResources.end()) {
            // Don't throw error, since multiple shader sources must have the same name.
            // Find both vertex and fragment shader sources:
            std::filesystem::path vertexShaderPath =
                path.parent_path() / (name + ".v.glsl");
            std::filesystem::path fragmentShaderPath =
                path.parent_path() / (name + ".f.glsl");

            if(std::filesystem::exists(vertexShaderPath) &&
               std::filesystem::exists(fragmentShaderPath)) {
                mShaderResources.insert(
                    {name,
                     ShaderResource::create(name, vertexShaderPath, fragmentShaderPath)});
            } else {
                Log::error() << "Failed to load shader '" << path.string() << "': "
                             << "could not find matching vertex/fragment shader!";
                return false;
            }
        }
    }

    if(alreadyExists) {
        Log::error() << "Cannot load " << resourceType << " resource '" << path.string()
                     << "': a " << resourceType << " resource with the name '" << name
                     << "' already exists!";
        return false;
    }
    return true;
}
