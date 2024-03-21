#pragma once

#include <vector>
#include <string>
#include <memory>
#include <tiny_obj_loader.h>

#include "GPUBuffer.hpp"

class ObjResource;
class ObjMesh {
public:
    using Ptr = std::shared_ptr<ObjMesh>;
    using CPtr = std::shared_ptr<const ObjMesh>;

    std::string name;
    ObjResource& parent;
    GPUBuffer vertexIndexBuffer;
    GPUBuffer normalIndexBuffer;
    GPUBuffer texCoordIndexBuffer;
    std::vector<int> materialIds; // Per-face material ID

    static Ptr create(ObjResource& parent, const tinyobj::shape_t& shape) {
        return std::make_shared<ObjMesh>(parent, shape);
    }

    ObjMesh(ObjResource& parent, const tinyobj::shape_t& shape);
};