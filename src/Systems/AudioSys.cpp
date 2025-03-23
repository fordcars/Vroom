#include "AudioSys.hpp"

#include <memory>

#include "Log.hpp"

// Static
AudioSys& AudioSys::get() {
    static std::unique_ptr<AudioSys> instance = std::make_unique<AudioSys>();
    return *instance;
}

AudioSys::AudioSys() : mMiniAudioEngine(std::make_unique<ma_engine>()) {}

AudioSys::~AudioSys() { ma_engine_uninit(mMiniAudioEngine.get()); }

bool AudioSys::init() {
    ma_result result = ma_engine_init(NULL, mMiniAudioEngine.get());
    if(result != MA_SUCCESS) {
        Log::error() << "Failed to initialize audio engine: "
                     << ma_result_description(result);
        return false;
    }

    return true;
}

void AudioSys::update(float) {}