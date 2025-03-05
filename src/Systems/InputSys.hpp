#pragma once
#include <SDL2/SDL.h>

#include <unordered_map>
#include <unordered_set>

class InputSys {
public:
    static InputSys& get();
    InputSys() = default;
    void init();
    void update();
    void handleEvent(const SDL_Event& event);

    enum class InputNeed { Jump, WalkLeft, WalkRight, WalkForward, WalkBackward, Crouch };

private:
    std::unordered_map<SDL_Keycode, InputNeed> mInputMapping;
    std::unordered_set<SDL_Keycode> mHeldKeys;
    void handleNeed(InputNeed need, bool isKeyDown);
    void handleMouseInput(const SDL_Event& event);
    void handleDownNeed(InputNeed need);
    void handleUpNeed(InputNeed need);
    void handlePressNeed(InputNeed need);
    void handleHoldNeed(InputNeed need);

    InputSys(const InputSys&) = delete;
    InputSys& operator=(const InputSys&) = delete;
    InputSys(InputSys&&) = delete;
    InputSys& operator=(InputSys&&) = delete;
};
