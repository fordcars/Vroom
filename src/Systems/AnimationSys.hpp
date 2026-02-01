#pragma once

#include "Components/AnimationComp.hpp"
#include "Components/RenderableComp.hpp"

class AnimationSys {
  public:
    static AnimationSys &get();
    AnimationSys() = default;
    void update(float deltaTime);

  private:
    static void applyAnimationChannels(AnimationComp &animationComp, Animation &currentAnim, float blendFactor,
                                       float deltaTime);
    static void updateAnimation(RenderableComp &renderableComp, AnimationComp &animationComp, float deltaTime);
    static void updateMeshTransforms(const ObjResource &objResource,
                                     std::unordered_map<AnimationNode *, glm::mat4> &cachedTransforms);
};
