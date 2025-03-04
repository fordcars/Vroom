#include "ObjMesh.hpp"

ObjMesh::ObjMesh(ObjResource& parent, const std::string& name,
                 const std::vector<unsigned int>& indices, AnimationNode* node)
    : name(name), parent(parent), node(node) {
    indexBuffer.setData<unsigned int>(GL_ELEMENT_ARRAY_BUFFER, indices);
    if(node) updateMeshTransform();
}

void ObjMesh::updateMeshTransform() {
    // Traverse the tree from bottom to top, applying each node transformation. Handles
    // "skeleton" transformations (not the skinning part).
    if(!this->node) return;

    transform = glm::mat4(1.0f);

    AnimationNode* joint = this->node;

    while(joint != nullptr) {
        glm::mat4 localTransform = glm::mat4(1.0f);

        localTransform = glm::translate(glm::mat4(1.0f), joint->translation) *
                         glm::mat4_cast(joint->rotation) *
                         glm::scale(glm::mat4(1.0f), joint->scale);

        // Apply from child to root
        transform = localTransform * transform;
        joint = joint->parent;
    }
}
