#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>

class GameplaySys {
public:
    static GameplaySys& get();
    GameplaySys() = default;
    void start();
    void update(float deltaTime);

private:
    GameplaySys(const GameplaySys&) = delete;
    GameplaySys& operator=(const GameplaySys&) = delete;
    GameplaySys(GameplaySys&&) = delete;
    GameplaySys& operator=(GameplaySys&&) = delete;
};
