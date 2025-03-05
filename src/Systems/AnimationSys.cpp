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

void AnimationSys::applyAnimationChannels(AnimationComp& animationComp,
                                          Animation& currentAnim, float blendFactor,
                                          float deltaTime) {
    for(auto& channel : currentAnim.getChannels()) {
        auto* node = channel.targetNode;
        if(!node || channel.sampler.timestamps.empty()) continue;

        // Find the two closest keyframes
        auto it =
            std::lower_bound(channel.sampler.timestamps.begin(),
                             channel.sampler.timestamps.end(), animationComp.currentTime);
        size_t i = std::distance(channel.sampler.timestamps.begin(), it);

        if(i == 0) {
            i = 1;
        } else if(i >= channel.sampler.timestamps.size()) {
            i = channel.sampler.timestamps.size() - 1;
        }

        float t0 = channel.sampler.timestamps[i - 1];
        float t1 = channel.sampler.timestamps[i];
        const glm::vec4& v0 = channel.sampler.values[i - 1];
        const glm::vec4& v1 = channel.sampler.values[i];

        // Avoid division by zero (if two keyframes have the same time)
        float alpha = (t1 > t0) ? (animationComp.currentTime - t0) / (t1 - t0) : 0.0f;

        if(channel.targetPath == Animation::TargetPath::Translation) {
            node->translation = lerpVec3(v0, v1, alpha) * blendFactor +
                                node->translation * (1.0f - blendFactor);
        } else if(channel.targetPath == Animation::TargetPath::Rotation) {
            glm::quat q0 = glm::quat(v0.w, v0.x, v0.y, v0.z);
            glm::quat q1 = glm::quat(v1.w, v1.x, v1.y, v1.z);
            node->rotation = slerpQuat(q0, q1, alpha) * blendFactor +
                             node->rotation * (1.0f - blendFactor);
        } else if(channel.targetPath == Animation::TargetPath::Scale) {
            node->scale = lerpVec3(v0, v1, alpha) * blendFactor +
                          node->scale * (1.0f - blendFactor);
        }
    }
}

void AnimationSys::updateAnimation(RenderableComp& renderableComp,
                                   AnimationComp& animationComp, float deltaTime) {
    // Detect animation change and initialize crossfade
    if(animationComp.currentAnimation != animationComp.previousAnimation) {
        animationComp.previousAnimation = animationComp.currentAnimation;
        animationComp.crossfadeTime = animationComp.crossfadeDuration;
    }

    if(!renderableComp.objectResource->animationContainer) return;

    auto animationContainer = renderableComp.objectResource->animationContainer;
    auto currentAnim = animationContainer->getAnimation(animationComp.currentAnimation);
    if(!currentAnim) return;

    float scaledDelta = deltaTime * animationComp.speed;
    animationComp.currentTime += scaledDelta;

    if(animationComp.startTime >= 0.0f &&
       animationComp.currentTime < animationComp.startTime) {
        animationComp.currentTime = animationComp.startTime;
    }

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

    if(animationComp.endTime >= 0.0f &&
       animationComp.currentTime > animationComp.endTime) {
        animationComp.currentTime = animationComp.endTime;
        if(animationComp.mode == AnimationMode::OneShot) {
            return;
        }
    }

    // During crossfade, interpolate between old and new animations
    if(animationComp.crossfadeTime > 0.0f) {
        float blendFactor =
            1.0f - (animationComp.crossfadeTime / animationComp.crossfadeDuration);
        animationComp.crossfadeTime -= deltaTime;
        applyAnimationChannels(animationComp, *currentAnim, blendFactor, deltaTime);
    } else {
        applyAnimationChannels(animationComp, *currentAnim, 1.0f, deltaTime);
    }

    // Update mesh and skin transforms with new node transforms
    for(auto& mesh : renderableComp.objectResource->objMeshes) {
        mesh->updateMeshTransform();
    }

    for(auto& skin : animationContainer->getSkins()) {
        skin->updateTransformBuffer();
    }
}
