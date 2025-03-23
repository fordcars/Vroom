#include "AudioResource.hpp"

#include "Systems/AudioSys.hpp"

AudioResource::AudioResource(const std::filesystem::path& path)
    : mSound(std::make_unique<ma_sound>()) {
    ma_result result = ma_sound_init_from_file(AudioSys::get().getEngine(), path.string().c_str(),
                                     0, NULL, NULL, mSound.get());
    if(result != MA_SUCCESS) {
        Log::error() << "Failed to load sound from file: "
                     << ma_result_description(result);
    }
}

AudioResource::~AudioResource() { ma_sound_uninit(mSound.get()); }

AudioResource::AudioResource(AudioResource&& other) noexcept { swap(*this, other); }

AudioResource& AudioResource::operator=(AudioResource&& other) noexcept {
    swap(*this, other);
    return *this;
}

void swap(AudioResource& first, AudioResource& second) noexcept {
    std::swap(first.mSound, second.mSound);
}
