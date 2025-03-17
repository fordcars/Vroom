#include "ObjMesh.hpp"

ObjMesh::ObjMesh(ObjResource& parent, const std::string& name,
                 std::vector<unsigned int> indices)
    : name(name), parent(parent), indices(std::move(indices)) {
    indexBuffer.setData<unsigned int>(GL_ELEMENT_ARRAY_BUFFER, this->indices);
}
