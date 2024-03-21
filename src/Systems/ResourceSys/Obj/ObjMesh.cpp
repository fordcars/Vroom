#include "ObjMesh.hpp"

ObjMesh::ObjMesh(ObjResource& parent,
    const tinyobj::shape_t& shape)
    : name(shape.name)
    , parent(parent)
    , materialIds(shape.mesh.material_ids) {

    // This stuff could be optimized
    std::vector<unsigned int> vertexIndices;
    std::vector<unsigned int> normalIndices;
    std::vector<unsigned int> texCoordIndices;
    vertexIndices.reserve(shape.mesh.indices.size());
    normalIndices.reserve(shape.mesh.indices.size());
    texCoordIndices.reserve(shape.mesh.indices.size());

    for(const tinyobj::index_t& index : shape.mesh.indices) {
        vertexIndices.push_back(index.vertex_index);

        if(index.normal_index == -1) {
            normalIndices.push_back(index.vertex_index);
        } else {
            normalIndices.push_back(index.normal_index);
        }
        
        if(index.texcoord_index == -1) {
            texCoordIndices.push_back(index.vertex_index);
        } else {
            texCoordIndices.push_back(index.texcoord_index);
        }
    }

    vertexIndexBuffer.setData<unsigned int>(GL_ARRAY_BUFFER, vertexIndices);
    normalIndexBuffer.setData<unsigned int>(GL_ARRAY_BUFFER, normalIndices);
    texCoordIndexBuffer.setData<unsigned int>(GL_ARRAY_BUFFER, texCoordIndices);
}
