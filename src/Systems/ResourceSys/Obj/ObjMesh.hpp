#pragma once

#include <tiny_obj_loader.h>

#include <memory>
#include <string>
#include <vector>

#include "GPUBuffer.hpp"

class ObjResource;
class ObjMesh {
public:
    using Ptr = std::shared_ptr<ObjMesh>;
    using CPtr = std::shared_ptr<const ObjMesh>;

    std::string name;
    ObjResource& parent;
    GPUBuffer indexBuffer;

    static Ptr create(ObjResource& parent, const std::string& name,
                      const std::vector<unsigned int>& indices) {
        return std::make_shared<ObjMesh>(parent, name, indices);
    }

    ObjMesh(ObjResource& parent, const std::string& name,
            const std::vector<unsigned int>& indices);
};