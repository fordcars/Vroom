#pragma once
#include <filesystem>
#include <unordered_map>
#include <string>

#include "Systems/System.hpp"
#include "ObjResource.hpp"

class ResourceSys : protected System<> {
public:
    bool loadResources();

private:
    std::unordered_map<std::string, ObjResource> mObjResources;

    bool loadResourcesFromDir(const std::filesystem::path& dirPath);
    bool loadResource(const std::filesystem::path& path);
};
