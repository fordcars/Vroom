#pragma once
#include <glm/glm.hpp>

#include "Systems/ResourceSys/Obj/GPUBuffer.hpp"
#include "Systems/ResourceSys/ShaderResource.hpp"

struct LightComp {
    struct Vertex {
        glm::vec3 position;
        glm::vec2 texcoord;
    };

    ShaderResource::CPtr shader;
    GPUBuffer vertexBuffer;
    std::size_t vertexCount = 6; // 2 triangles

    glm::vec3 diffuse = {1.0f, 1.0f, 1.0f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    LightComp() {
        // Fill vertex buffer with full screen quad (2 triangles)
        vertexBuffer.setData<Vertex>(GL_ARRAY_BUFFER, {
                                                          {{-1, -1, 0}, {0, 0}},
                                                          {{1, -1, 0}, {1, 0}},
                                                          {{1, 1, 0}, {1, 1}},

                                                          {{-1, -1, 0}, {0, 0}},
                                                          {{1, 1, 0}, {1, 1}},
                                                          {{-1, 1, 0}, {0, 1}},
                                                      });
    }
};
