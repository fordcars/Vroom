#pragma once
#include <filesystem>
#include <unordered_map>
#include <string>

#include "Obj/ObjResource.hpp"
#include "ShaderResource.hpp"

class ResourceSys {
public:
    static ResourceSys& get();
    
    bool loadResources();
    ObjResource::CPtr getObjResource(const std::string& name) const;

private:
    std::unordered_map<std::string, ObjResource::Ptr> mObjResources;
    std::unordered_map<std::string, ShaderResource::Ptr> mShaderResources;

    bool loadResourcesFromDir(const std::filesystem::path& dirPath);
    bool loadResource(const std::filesystem::path& path);
};
