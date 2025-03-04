#include "AnimationSys.hpp"

#include <glm/gtc/quaternion.hpp>

#include "ResourceSys/Obj/Animation/Animation.hpp"

namespace {
// Interpolation helper
glm::vec3 lerpVec3(const glm::vec3& a, const glm::vec3& b, float t) {
    return a * (1.0f - t) + b * t;
}

glm::quat slerpQuat(const glm::quat& a, const glm::quat& b, float t) {
    return glm::slerp(a, b, t);
}
} // namespace

// Static
AnimationSys& AnimationSys::get() {
    static std::unique_ptr<AnimationSys> instance = std::make_unique<AnimationSys>();
    return *instance;
}

void AnimationSys::update(float deltaTime) {
    EntityFilter<RenderableComp, AnimationComp> filter;
    for(auto& [renderableComp, animationComp] : filter) {
        updateAnimation(renderableComp, animationComp, deltaTime);
    }
}

void AnimationSys::updateAnimation(RenderableComp& renderableComp,
                                   AnimationComp& animationComp, float deltaTime) {
    if(!renderableComp.objectResource->animationContainer) return;

    auto animationContainer = renderableComp.objectResource->animationContainer;
    auto currentAnim = animationContainer->getAnimation(animationComp.currentAnimation);
    if(!currentAnim) return;

    animationComp.currentTime += deltaTime;

    if(animationComp.mode == AnimationMode::OneShot) {
        if(animationComp.currentTime > currentAnim->getDuration()) {
            animationComp.currentTime = currentAnim->getDuration();
            return; // Stop updating if the animation is one shot and has finished
        }
    } else if(animationComp.mode == AnimationMode::Loop) {
        if(animationComp.currentTime > currentAnim->getDuration()) {
            animationComp.currentTime =
                fmod(animationComp.currentTime, currentAnim->getDuration());
        }
    }

    for(auto& channel : currentAnim->getChannels()) {
        auto* node = channel.targetNode;
        if(!node || channel.sampler.timestamps.empty()) continue;

        // Find the two closest keyframes
        auto it =
            std::lower_bound(channel.sampler.timestamps.begin(),
                             channel.sampler.timestamps.end(), animationComp.currentTime);
        size_t i = std::distance(channel.sampler.timestamps.begin(), it);

        if(i == 0) {
            i = 1; // Ensure there's a previous keyframe
        }
        if(i == channel.sampler.timestamps.size()) {
            continue; // Skip
        }

        float t0 = channel.sampler.timestamps[i - 1];
        float t1 = channel.sampler.timestamps[i];
        float alpha = (animationComp.currentTime - t0) /
                      (t1 - t0); // Normalized time between keyframes

        if(channel.targetPath == Animation::TargetPath::Translation) {
            glm::vec3 v0 = glm::vec3(channel.sampler.values[i - 1]);
            glm::vec3 v1 = glm::vec3(channel.sampler.values[i]);
            node->translation = lerpVec3(v0, v1, alpha);
        } else if(channel.targetPath == Animation::TargetPath::Rotation) {
            glm::quat q0 = glm::quat(channel.sampler.values[i - 1]);
            glm::quat q1 = glm::quat(channel.sampler.values[i]);
            node->rotation = slerpQuat(q0, q1, alpha);
        } else if(channel.targetPath == Animation::TargetPath::Scale) {
            glm::vec3 s0 = glm::vec3(channel.sampler.values[i - 1]);
            glm::vec3 s1 = glm::vec3(channel.sampler.values[i]);
            node->scale = lerpVec3(s0, s1, alpha);
        }
    }

    for(auto& skeleton : animationContainer->getSkeletons()) {
        skeleton->updateTransformBuffer();
    }
}
