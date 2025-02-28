#pragma once
#include <filesystem>
#include <unordered_map>
#include <string>

#include "Obj/ObjResource.hpp"
#include "ShaderResource.hpp"

class ResourceSys {
public:
    static ResourceSys& get();
    ResourceSys() = default;
    
    bool loadResources();
    ObjResource::CPtr getObjResource(const std::string& name) const;
    ShaderResource::CPtr getShaderResource(const std::string& name) const;

private:
    std::unordered_map<std::string, ObjResource::Ptr> mObjResources;
    std::unordered_map<std::string, ShaderResource::Ptr> mShaderResources;

    ResourceSys(const ResourceSys&) = delete;
    ResourceSys& operator=(const ResourceSys&) = delete;
    ResourceSys(ResourceSys&&) = delete;
    ResourceSys& operator=(ResourceSys&&) = delete;

    bool loadResourcesFromDir(const std::filesystem::path& dirPath);
    bool loadResource(const std::filesystem::path& path);
};
