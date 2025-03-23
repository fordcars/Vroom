#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>

#include "Log.hpp"
#include "miniaudio.h"
#include "Systems/AudioSys.hpp"

class AudioResource {
public:
    using Ptr = std::shared_ptr<AudioResource>;
    using CPtr = std::shared_ptr<const AudioResource>;

    static Ptr create(const std::filesystem::path& path) {
        return std::make_shared<AudioResource>(path);
    }

    AudioResource(const std::filesystem::path& path);
    ~AudioResource();
    AudioResource(const AudioResource& other) = delete; // Don't copy
    AudioResource(AudioResource&& other) noexcept;
    AudioResource& operator=(AudioResource other) = delete; // Same
    AudioResource& operator=(AudioResource&& other) noexcept;
    friend void swap(AudioResource& first, AudioResource& second) noexcept;

    ma_sound* getSound() { return mMiniAudioSound.get(); }
    const ma_sound* getSound() const { return mMiniAudioSound.get(); }

private:
    // AudioSys::Engine must be referred to through a shared_ptr for proper destruction order,
    // since the ma_engine is used by ma_sounds during their lifetime and uninitialization.
    // Its inclusion here is to ensure the proper dependency tree.
    AudioSys::Engine::Ptr mEngine;
    std::unique_ptr<ma_sound> mMiniAudioSound;

public:
    template <
        auto MiniAudioFunction, typename... Args,
        typename = std::enable_if_t<std::is_same<
            decltype(MiniAudioFunction(mMiniAudioSound.get(), std::declval<Args>()...)),
            ma_result>::value>>
    bool call(Args&&... args) {
        ma_result result =
            MiniAudioFunction(mMiniAudioSound.get(), std::forward<Args>(args)...);
        if(result != MA_SUCCESS) {
            Log::warn() << "Failed to call miniaudio sound function: "
                        << ma_result_description(result);
            return false;
        }
        return true;
    }

    template <auto MiniAudioFunction, typename... Args,
    typename = std::enable_if_t<!std::is_same<
            decltype(MiniAudioFunction(mMiniAudioSound.get(), std::declval<Args>()...)),
            ma_result>::value>>
    decltype(auto) call(Args&&... args) {
        return MiniAudioFunction(mMiniAudioSound.get(), std::forward<Args>(args)...);
    }
};
