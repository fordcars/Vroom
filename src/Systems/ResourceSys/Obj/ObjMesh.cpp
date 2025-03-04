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

        // ✅ Correct transformation order: Rotation → Translation → Scale
        localTransform = glm::translate(glm::mat4(1.0f),
                                        joint->translation) * // Move to correct position
                         glm::mat4_cast(joint->rotation) *    // Rotate in place
                         glm::scale(glm::mat4(1.0f), joint->scale); // Scale in place

        // ✅ Apply from child to root (ensures proper accumulation)
        transform = localTransform * transform;

        joint = joint->parent;
    }
}
