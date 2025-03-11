#include "ObjMesh.hpp"

ObjMesh::ObjMesh(ObjResource& parent, const std::string& name,
                 const std::vector<unsigned int>& indices, AnimationNode* animationNode,
                 Skin::Ptr skin)
    : name(name), parent(parent), animationNode(animationNode), skin(skin) {
    indexBuffer.setData<unsigned int>(GL_ELEMENT_ARRAY_BUFFER, indices);
}
