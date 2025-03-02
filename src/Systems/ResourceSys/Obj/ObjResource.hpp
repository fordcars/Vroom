#pragma once

#include <tiny_obj_loader.h>

#include <filesystem>
#include <memory>
#include <vector>

#include "GPUBuffer.hpp"
#include "ObjAnimation.hpp"
#include "ObjMesh.hpp"

class ObjResource {
public:
    using Ptr = std::shared_ptr<ObjResource>;
    using CPtr = std::shared_ptr<const ObjResource>;

    GPUBuffer vertexBuffer;
    GPUBuffer normalBuffer;
    GPUBuffer texcoordBuffer;
    GPUBuffer materialIdBuffer;
    GPUBuffer materialUniformBuffer;
    std::vector<ObjMesh::Ptr> objMeshes;
    ObjAnimation::Ptr animation;

    static Ptr create(const std::filesystem::path& path) {
        return std::make_shared<ObjResource>(path);
    }

    ObjResource(const std::filesystem::path& path);

private:
    std::filesystem::path mPath;

    bool load();
    void loadOnGPU(const tinyobj::ObjReader& reader);
    void loadMeshes(const tinyobj::ObjReader& reader);
};