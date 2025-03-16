#pragma once
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>

class InputSys {
public:
    static InputSys& get();
    InputSys() = default;
    bool init();
    void update(float deltaTime);
    void handleEvent(const SDL_Event& event);

    enum class InputNeed {
        Jump,
        WalkLeft,
        WalkRight,
        WalkForward,
        WalkBackward,
        Run,
        Crouch,

        // Debug stuff
        ToggleShowFPS,
        ChangeDebugRenderMode,
        ToggleShowWalkVectors,
    };

private:
    std::unordered_map<SDL_Keycode, InputNeed> mInputMapping;
    std::unordered_set<SDL_Keycode> mHeldKeys;

    glm::vec3 mWalkDirection{}; // Walk direction for current step
    bool mRunning = false;
    int mDebugRenderMode = 0;
    bool mShowDebugWalkVectors = false;

    void handleNeed(InputNeed need, bool isKeyDown);
    void handleDownNeed(InputNeed need);
    void handleUpNeed(InputNeed need);
    void handlePressNeed(InputNeed need);
    void handleHoldNeed(InputNeed need);

    void handleMouseInput(const SDL_Event& event);
    void handleWalking(float deltaTime);
    void cycleDebugRenderMode();

    InputSys(const InputSys&) = delete;
    InputSys& operator=(const InputSys&) = delete;
    InputSys(InputSys&&) = delete;
    InputSys& operator=(InputSys&&) = delete;
};
