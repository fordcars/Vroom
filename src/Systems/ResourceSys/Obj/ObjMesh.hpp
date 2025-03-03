#pragma once

#include <tiny_obj_loader.h>

#include <glm/glm.hpp>
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
    glm::mat4 transform{1.0f};

    static Ptr create(ObjResource& parent, const std::string& name,
                      const std::vector<unsigned int>& indices,
                      const glm::mat4& transform = glm::mat4(1.0f)) {
        return std::make_shared<ObjMesh>(parent, name, indices, transform);
    }

    ObjMesh(ObjResource& parent, const std::string& name,
            const std::vector<unsigned int>& indices,
            glm::mat4 transform = glm::mat4(1.0f));
};