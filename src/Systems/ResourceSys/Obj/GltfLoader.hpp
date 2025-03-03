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
    void loadMesh(ObjResource& resource, std::vector<ObjResource::Vertex>& outVertices,
                  const tinygltf::Model& model, const tinygltf::Mesh& mesh,
                  const glm::mat4& meshTransform);
    glm::mat4 computeNodeTransform(const tinygltf::Node& node, glm::mat4 parentTransform);
    void loadMaterials(ObjResource& resource, const tinygltf::Model& model);
    void loadAnimation(ObjResource& resource, const tinygltf::Model& model);
};
