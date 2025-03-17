#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class ObjResource;
class ObjOrientedBoundingBox {
public:
    using Ptr = std::shared_ptr<ObjOrientedBoundingBox>;
    using CPtr = std::shared_ptr<const ObjOrientedBoundingBox>;

    glm::vec3 minCorner {};
    glm::vec3 maxCorner {};

    static Ptr create(const ObjResource& resource) {
        return std::make_shared<ObjOrientedBoundingBox>(resource);
    }

    ObjOrientedBoundingBox(const ObjResource& resource);

private:
    void calculateCorners(const ObjResource& resource);
};