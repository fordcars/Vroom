#pragma once
#include <SDL2/SDL.h>

#include <unordered_map>

class InputSys {
public:
    static InputSys& get();
    InputSys() = default;
    void init();
    void handleEvent(const SDL_Event& event);

    enum class InputNeed { Jump, WalkLeft, WalkRight, Crouch };

private:
    std::unordered_map<SDL_Keycode, InputNeed> mInputMapping;
    void handleNeed(InputNeed need, bool isKeyDown);
    void handleMouseInput(const SDL_Event& event);

    InputSys(const InputSys&) = delete;
    InputSys& operator=(const InputSys&) = delete;
    InputSys(InputSys&&) = delete;
    InputSys& operator=(InputSys&&) = delete;
};
