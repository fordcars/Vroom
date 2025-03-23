#include "AudioSys.hpp"

#include <memory>
#include <optional>

#include "Log.hpp"
#include "Entities/EntityFilter.hpp"
#include "Components/PositionComp.hpp"
#include "Components/SoundComp.hpp"
#include "Components/PhysicsComp.hpp"
#include "Entities/CameraEntity.hpp"

// Static
AudioSys& AudioSys::get() {
    static std::unique_ptr<AudioSys> instance = std::make_unique<AudioSys>();
    return *instance;
}

AudioSys::AudioSys() : mEngine(std::make_shared<Engine>()) {}

bool AudioSys::init() {
    ma_result result = ma_engine_init(NULL, &mEngine->miniAudioEngine);
    if(result != MA_SUCCESS) {
        Log::error() << "Failed to initialize audio engine: "
                     << ma_result_description(result);
        return false;
    }

    return true;
}

void AudioSys::update(float) {
    // Update spatial audio
    auto cameraPos = CameraEntity::instances[0].get<PositionComp>();
    auto cameraInfo = CameraEntity::instances[0].get<CameraInfoComp>();
    call<ma_engine_listener_set_position>(0, cameraPos.coords.x, cameraPos.coords.y, cameraPos.coords.z);
    
    glm::vec3 dir = cameraInfo.direction.w == 1.0f
        ? glm::vec3(cameraInfo.direction) - cameraPos.coords
        : glm::vec3(cameraInfo.direction);

    call<ma_engine_listener_set_direction>(0, dir.x, dir.y, dir.z);
    call<ma_engine_listener_set_world_up>(0, cameraInfo.upVector.x, cameraInfo.upVector.y, cameraInfo.upVector.z);

    // Update spatial parameters for all entities
    for(auto& [sound, position, physics] : EntityFilter<SoundComp, PositionComp, std::optional<PhysicsComp>>()) {
        if(!sound.audioResource || !sound.audioResource->call<ma_sound_is_spatialization_enabled>()) {
            continue;
        }
        sound.audioResource->call<ma_sound_set_position>(position.coords.x, position.coords.y, position.coords.z);
        sound.audioResource->call<ma_sound_set_direction>(sound.direction.x, sound.direction.y, sound.direction.z);
        sound.audioResource->call<ma_sound_set_cone>(sound.coneInnerAngle, sound.coneOuterAngle, sound.coneOuterGain);
        sound.audioResource->call<ma_sound_set_rolloff>(sound.rolloffFactor);

        if(physics) {
            auto& physicsComp = physics->get();
            sound.audioResource->call<ma_sound_set_velocity>(physicsComp.velocity.x, physicsComp.velocity.y, physicsComp.velocity.z);
        }
    }
}