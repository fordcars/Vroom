#pragma once
#include <filesystem>
#include <unordered_map>
#include <string>

#include "Obj/ObjResource.hpp"

class ResourceSys {
public:
    bool loadResources();

private:
    std::unordered_map<std::string, ObjResource::Ptr> mObjResources;

    bool loadResourcesFromDir(const std::filesystem::path& dirPath);
    bool loadResource(const std::filesystem::path& path);
    ObjResource::CPtr getObjResource(const std::string& name) const;
};
