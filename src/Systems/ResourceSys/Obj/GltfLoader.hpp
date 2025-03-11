#pragma once

#include <tiny_gltf.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

#include "ObjLoader.hpp"
#include "ObjResource.hpp"

class GltfLoader : public ObjLoader {
public:
    GltfLoader(const std::filesystem::path& path);
    bool load(ObjResource& resource) override final;

private:
    const std::filesystem::path mPath;

    void loadMeshes(ObjResource& resource, const tinygltf::Model& model);
    void loadPrimitives(ObjResource& resource,
                        std::vector<ObjResource::Vertex>& outVertices,
                        const tinygltf::Model& model, int gltfNodeIndex,
                        int gltfSkinIndex);
    void loadMaterials(ObjResource& resource, const tinygltf::Model& model);
};
