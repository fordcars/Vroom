#pragma once

#include <filesystem>
#include <vector>
#include <tiny_obj_loader.h>
#include "Resource.hpp"

class ObjResource : public Resource {
public:
    ObjResource(const std::string& name, const std::filesystem::path& path);
    const tinyobj::attrib_t& getAttrib() const;
    const std::vector<tinyobj::shape_t>& getShapes() const;
    const std::vector<tinyobj::material_t>& getMaterials() const;

private:
    std::filesystem::path mPath;
    tinyobj::ObjReader mReader;
    bool load();
};