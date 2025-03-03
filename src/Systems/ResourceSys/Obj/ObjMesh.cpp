#include "ObjMesh.hpp"

ObjMesh::ObjMesh(ObjResource& parent, const std::string& name,
                 const std::vector<unsigned int>& indices, glm::mat4 transform)
    : name(name), parent(parent), transform(std::move(transform)) {
    indexBuffer.setData<unsigned int>(GL_ELEMENT_ARRAY_BUFFER, indices);
}
