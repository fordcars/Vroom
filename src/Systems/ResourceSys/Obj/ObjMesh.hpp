#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Animation/AnimationNode.hpp"
#include "Animation/Skin.hpp"
#include "GPUBuffer.hpp"

class ObjResource;
class ObjMesh {
public:
    using Ptr = std::shared_ptr<ObjMesh>;
    using CPtr = std::shared_ptr<const ObjMesh>;

    std::string name;
    ObjResource& parent;
    GPUBuffer indexBuffer;
    glm::mat4 transform{1.0f};
    AnimationNode* animationNode = nullptr; // Optional
    Skin::Ptr skin;                         // Optional

    static Ptr create(ObjResource& parent, const std::string& name,
                      const std::vector<unsigned int>& indices,
                      AnimationNode* animationNode = nullptr, Skin::Ptr skin = nullptr) {
        return std::make_shared<ObjMesh>(parent, name, indices, animationNode, skin);
    }

    ObjMesh(ObjResource& parent, const std::string& name,
            const std::vector<unsigned int>& indices,
            AnimationNode* animationNode = nullptr, Skin::Ptr skin = nullptr);
};