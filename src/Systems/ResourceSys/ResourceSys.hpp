#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>

#include "AudioResource.hpp"
#include "Obj/ObjResource.hpp"
#include "ShaderResource.hpp"

class ResourceSys {
public:
    static ResourceSys& get();
    ResourceSys() = default;
    ResourceSys(const ResourceSys&) = delete;
    ResourceSys& operator=(const ResourceSys&) = delete;
    ResourceSys(ResourceSys&&) = delete;
    ResourceSys& operator=(ResourceSys&&) = delete;

    bool loadResources();
    ObjResource::Ptr getObjResource(const std::string& name);
    ShaderResource::CPtr getShaderResource(const std::string& name) const;
    AudioResource::Ptr getAudioResource(const std::string& name);

private:
    std::unordered_map<std::string, ObjResource::Ptr> mObjResources;
    std::unordered_map<std::string, ShaderResource::Ptr> mShaderResources;
    std::unordered_map<std::string, AudioResource::Ptr> mAudioResources;

    bool loadResourcesFromDir(const std::filesystem::path& dirPath);
    bool loadResource(const std::filesystem::path& path);
};
