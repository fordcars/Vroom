#include "AnimationSys.hpp"

#include <glm/gtc/quaternion.hpp>

#include "Components/AnimationComp.hpp" // Include the header for AnimationComp

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
    EntityFilter<AnimationComp> filter;
    for(auto& [animation] : filter) {
        updateAnimation(animation, deltaTime);
    }
}

void AnimationSys::updateAnimation(AnimationComp& animation, float deltaTime) {
    animation.currentTime += deltaTime;

    if(animation.mode == AnimationMode::OneShot) {
        if(animation.currentTime > animation.objAnimation.duration) {
            animation.currentTime = animation.objAnimation.duration;
            return; // Stop updating if the animation is one shot and has finished
        }
    } else if(animation.mode == AnimationMode::Loop) {
        if(animation.currentTime > animation.objAnimation.duration) {
            animation.currentTime =
                fmod(animation.currentTime, animation.objAnimation.duration);
        }
    }

    auto& nodes = animation.objAnimation.nodes;
    auto& animationChannels = animation.objAnimation.animationChannels;

    for(auto& channel : animationChannels) {
        auto& node = nodes[channel.targetNode];

        // Find the two closest keyframes
        auto it =
            std::lower_bound(channel.sampler.timestamps.begin(),
                             channel.sampler.timestamps.end(), animation.currentTime);
        size_t i = std::distance(channel.sampler.timestamps.begin(), it);

        if(i == 0) {
            i = 1; // Ensure there's a previous keyframe
        } else if(i == channel.sampler.timestamps.size()) {
            i = channel.sampler.timestamps.size() - 1; // Ensure there's a next keyframe
        }

        float t0 = channel.sampler.timestamps[i - 1];
        float t1 = channel.sampler.timestamps[i];
        float alpha =
            (animation.currentTime - t0) / (t1 - t0); // Normalized time between keyframes

        if(channel.targetPath == "translation") {
            glm::vec3 v0 = glm::vec3(channel.sampler.values[i]);
            glm::vec3 v1 = glm::vec3(channel.sampler.values[i + 1]);
            node.translation = lerpVec3(v0, v1, alpha);
        } else if(channel.targetPath == "rotation") {
            glm::quat q0 = glm::quat(channel.sampler.values[i]);
            glm::quat q1 = glm::quat(channel.sampler.values[i + 1]);
            node.rotation = slerpQuat(q0, q1, alpha);
        } else if(channel.targetPath == "scale") {
            glm::vec3 s0 = glm::vec3(channel.sampler.values[i]);
            glm::vec3 s1 = glm::vec3(channel.sampler.values[i + 1]);
            node.scale = lerpVec3(s0, s1, alpha);
        }
    }

    animation.objAnimation.updateBoneBuffer();
}
