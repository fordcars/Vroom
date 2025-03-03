#pragma once

#include <memory>

#include "GPUBuffer.hpp"
#include "ObjAnimation.hpp"
#include "ObjLoader.hpp"
#include "ObjMesh.hpp"

class ObjResource {
public:
    using Ptr = std::shared_ptr<ObjResource>;
    using CPtr = std::shared_ptr<const ObjResource>;

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        unsigned int materialId;
    };

    GPUBuffer vertexBuffer;
    GPUBuffer materialUniformBuffer;
    ObjAnimation::Ptr animation;
    std::vector<ObjMesh::Ptr> objMeshes;

    static Ptr create(std::unique_ptr<ObjLoader> loader) {
        return std::make_shared<ObjResource>(std::move(loader));
    }

    ObjResource(std::unique_ptr<ObjLoader> loader);
};