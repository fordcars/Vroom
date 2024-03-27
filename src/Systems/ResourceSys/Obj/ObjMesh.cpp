#include "ObjMesh.hpp"

ObjMesh::ObjMesh(ObjResource& parent, const std::string& name,
    const std::vector<unsigned int>& indices)
    : name(name)
    , parent(parent) {
    indexBuffer.setData<unsigned int>(GL_ELEMENT_ARRAY_BUFFER, indices);
}
