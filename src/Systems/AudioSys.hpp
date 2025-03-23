#pragma once

#include <memory>

#include "miniaudio.h"

class AudioSys {
public:
    static AudioSys& get();
    AudioSys();
    ~AudioSys();

    bool init();
    void update(float deltaTime);
    ma_engine* getEngine() { return mMiniAudioEngine.get(); }
    const ma_engine* getEngine() const { return mMiniAudioEngine.get(); }

private:
    std::unique_ptr<ma_engine> mMiniAudioEngine;

    AudioSys(const AudioSys&) = delete;
    AudioSys& operator=(const AudioSys&) = delete;
    AudioSys(AudioSys&&) = delete;
    AudioSys& operator=(AudioSys&&) = delete;
};
