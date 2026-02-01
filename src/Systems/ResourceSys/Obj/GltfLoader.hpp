#pragma once

#include <tiny_gltf.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

#include "ObjLoader.hpp"
#include "ObjResource.hpp"

class GltfLoader : public ObjLoader {
public:
    GltfLoader(std::filesystem::path path);
    bool load(ObjResource &resource) final;

private:
    const std::filesystem::path mPath;

    static void loadMaterials(ObjResource &resource, const tinygltf::Model &model);
    static void loadImages(ObjResource &resource, const tinygltf::Model &model);
    static void loadTextures(ObjResource &resource, const tinygltf::Model &model);
    static void loadMeshes(ObjResource &resource, const tinygltf::Model &model);
    static void loadPrimitives(ObjResource &resource, std::vector<ObjResource::Vertex> &outVertices,
                               const tinygltf::Model &model, int gltfNodeIndex, int gltfSkinIndex,
                               const glm::mat4 &meshTransform);
    static void setMeshTextures(ObjResource &resource, ObjMesh &mesh, const tinygltf::Model &model,
                                const tinygltf::Primitive &primitive);
};
