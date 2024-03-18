#pragma once

#include <vector>
#include <string>
#include <memory>
#include <tiny_obj_loader.h>

#include "GPUBuffer.hpp"

class ObjResource;
class ObjMesh {
public:
    using Ptr = std::weak_ptr<ObjMesh>;
    using CPtr = std::weak_ptr<const ObjMesh>;

    static Ptr create(ObjResource& parent, const tinyobj::shape_t& shape) {
        return std::make_shared<ObjMesh>(parent, shape);
    }

    ObjMesh(ObjResource& parent, const tinyobj::shape_t& shape);
    ObjResource& getParent();
    const ObjResource& getParent() const;

private:
    ObjResource& mParent;
    std::string mName;
    GPUBuffer mIndexBuffer;
    std::vector<int> mMaterialIds;
};