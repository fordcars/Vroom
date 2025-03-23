#pragma once

#include <memory>

#include "Animation/AnimationContainer.hpp"
#include "GPUBuffer.hpp"
#include "ObjImage.hpp"
#include "ObjLoader.hpp"
#include "ObjMesh.hpp"
#include "ObjTexture.hpp"
#include "ObjBoundingBox.hpp"

class ObjResource {
public:
    using Ptr = std::shared_ptr<ObjResource>;
    using CPtr = std::shared_ptr<const ObjResource>;

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        unsigned int materialId;

        // Animation
        glm::uvec4 joints;
        glm::vec4 weights;
    };

    GPUBuffer vertexBuffer;
    GPUBuffer materialUniformBuffer;
    std::vector<Vertex> vertices; // Same vertex data as in vertexBuffer
    std::vector<ObjMesh::Ptr> objMeshes;
    std::vector<ObjImage::Ptr> objImages;
    std::vector<ObjTexture::Ptr> objTextures;
    ObjBoundingBox::Ptr boundingBox;
    AnimationContainer::Ptr animationContainer; // Optional

    static Ptr create(std::unique_ptr<ObjLoader> loader) {
        return std::make_shared<ObjResource>(std::move(loader));
    }

    ObjResource(std::unique_ptr<ObjLoader> loader);
};