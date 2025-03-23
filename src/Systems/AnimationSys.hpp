#pragma once

#include "Components/AnimationComp.hpp"
#include "Components/RenderableComp.hpp"
#include "Entities/EntityFilter.hpp"

class AnimationSys {
public:
    static AnimationSys& get();
    AnimationSys() = default;
    void update(float deltaTime);

private:
    void applyAnimationChannels(AnimationComp& animationComp, Animation& currentAnim,
                                float blendFactor, float deltaTime);
    void updateAnimation(RenderableComp& renderableComp, AnimationComp& animationComp,
                         float deltaTime);
    void updateMeshTransforms(const ObjResource& objResource, std::unordered_map<AnimationNode*, glm::mat4>& cachedTransforms);
};
