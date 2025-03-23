#pragma once

#include <memory>

#include "Log.hpp"
#include "miniaudio.h"

class AudioSys {
public:
    // AudioResources also depend on ma_engine internally; it refers to this struct
    // through a shared_ptr for proper destruction order.
    struct Engine {
        using Ptr = std::shared_ptr<Engine>;
        ma_engine miniAudioEngine{};

        Engine() = default;
        ~Engine() { ma_engine_uninit(&miniAudioEngine); }
        Engine(const AudioSys&) = delete;
        Engine& operator=(const AudioSys&) = delete;
        Engine(AudioSys&&) = delete;
        Engine& operator=(AudioSys&&) = delete;
    };

    static AudioSys& get();
    AudioSys();
    bool init();
    void update(float deltaTime);
    Engine::Ptr getEngine() { return mEngine; }

private:
    Engine::Ptr mEngine;

    AudioSys(const AudioSys&) = delete;
    AudioSys& operator=(const AudioSys&) = delete;
    AudioSys(AudioSys&&) = delete;
    AudioSys& operator=(AudioSys&&) = delete;

public:
    template <auto MiniAudioFunction, typename... Args,
              typename = std::enable_if_t<
                  std::is_same<decltype(MiniAudioFunction(&mEngine->miniAudioEngine,
                                                          std::declval<Args>()...)),
                               ma_result>::value>>
    bool call(Args&&... args) {
        ma_result result =
            MiniAudioFunction(&mEngine->miniAudioEngine, std::forward<Args>(args)...);
        if(result != MA_SUCCESS) {
            Log::warn() << "Failed to call miniaudio engine function: "
                        << ma_result_description(result);
            return false;
        }
        return true;
    }

    template <auto MiniAudioFunction, typename... Args>
    decltype(auto) call(Args&&... args) {
        MiniAudioFunction(&mEngine->miniAudioEngine, std::forward<Args>(args)...);
    }
};
