#include "ObjMesh.hpp"

ObjMesh::ObjMesh(ObjResource& parent,
    const tinyobj::shape_t& shape)
    : name(shape.name)
    , parent(parent)
    , materialIds(shape.mesh.material_ids) {
    indexBuffer.setData(shape.mesh.indices);
}
