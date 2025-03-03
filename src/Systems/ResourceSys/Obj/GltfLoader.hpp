#pragma once

#include <filesystem>

#include "ObjLoader.hpp"

class ObjResource;
class GltfLoader : public ObjLoader {
public:
    GltfLoader(const std::filesystem::path& path);
    bool load(ObjResource& resource) override final;

private:
    const std::filesystem::path mPath;

    void loadMeshes(ObjResource& resource, const tinygltf::Model& model);
    void loadMaterials(ObjResource& resource, const tinygltf::Model& model);
};
