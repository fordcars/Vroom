#pragma once

#include <tiny_obj_loader.h>

#include <filesystem>

#include "ObjLoader.hpp"

class ObjResource;
class WavefrontLoader : public ObjLoader {
public:
    WavefrontLoader(const std::filesystem::path& path);
    bool load(ObjResource& resource) override final;

private:
    const std::filesystem::path mPath;
    void loadOnGPU(ObjResource& resource, const tinyobj::ObjReader& reader);
    void loadMeshes(ObjResource& resource, const tinyobj::ObjReader& reader);
};
