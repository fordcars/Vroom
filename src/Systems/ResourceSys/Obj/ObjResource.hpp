#pragma once

#include <filesystem>
#include <vector>
#include <memory>

#include <tiny_obj_loader.h>
#include "Systems/ResourceSys/Resource.hpp"
#include "GPUBuffer.hpp"
#include "ObjMesh.hpp"

class ObjResource : public Resource {
public:
    using Ptr = std::weak_ptr<ObjResource>;
    using CPtr = std::weak_ptr<const ObjResource>;

    static Ptr create(const std::string& name, const std::filesystem::path& path) {
        return std::make_shared<ObjResource>(name, path);
    }

    ObjResource(const std::string& name, const std::filesystem::path& path);

private:
    std::filesystem::path mPath;

    // Buffers
    GPUBuffer mVertexBuffer;
    GPUBuffer mNormalBuffer;
    GPUBuffer mTexCoordBuffer;
    GPUBuffer mColorBuffer;
    std::vector<ObjMesh::Ptr> mObjMeshes;

    bool load();
    void loadOnGPU(const tinyobj::ObjReader& reader);
};