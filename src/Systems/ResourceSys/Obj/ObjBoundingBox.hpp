#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <tuple>

class ObjResource;
class ObjBoundingBox {
public:
    using Ptr = std::shared_ptr<ObjBoundingBox>;
    using CPtr = std::shared_ptr<const ObjBoundingBox>;

    // Modelspace AABB corners
    glm::vec3 minCorner {};
    glm::vec3 maxCorner {};

    static Ptr create(const ObjResource& resource) {
        return std::make_shared<ObjBoundingBox>(resource);
    }

    ObjBoundingBox(const ObjResource& resource);
    std::pair<glm::vec3, glm::vec3> getWorldspaceAABB(const glm::mat4& modelMatrix) const;

private:
    void calculateModelspaceAABB(const ObjResource& resource);
};