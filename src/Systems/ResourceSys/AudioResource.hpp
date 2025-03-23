#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "miniaudio.h"
#include "Log.hpp"

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

    template <auto MiniAudioFunction, typename... Args>
    bool run(Args&&... args) {
        ma_result result = MiniAudioFunction(mSound.get(), std::forward<Args>(args)...);
        if(result != MA_SUCCESS) {
            Log::warn() << "Failed to call miniaudio function: "
                        << ma_result_description(result);
            return false;
        }
        return true;
    }

    ma_sound* getSound() { return mSound.get(); }
    const ma_sound* getSound() const { return mSound.get(); }

private:
    std::unique_ptr<ma_sound> mSound;
};
