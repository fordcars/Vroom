#pragma once

#include <filesystem>
#include "Resource.hpp"

class ObjResource : public Resource {
public:
    ObjResource(const std::string& name, const std::filesystem::path& path);

private:
    std::filesystem::path mPath;
    bool load();
};