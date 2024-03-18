#include "ObjMesh.hpp"

ObjMesh::ObjMesh(ObjResource& parent,
    const tinyobj::shape_t& shape)
    : mParent(parent)
    , mName(shape.name)
    , mMaterialIds(shape.mesh.material_ids) {
    mIndexBuffer.setData(shape.mesh.indices);
}

ObjResource& ObjMesh::getParent() {
    return mParent;
}

const ObjResource& ObjMesh::getParent() const {
    return mParent;
}