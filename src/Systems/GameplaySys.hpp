#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>

class GameplaySys {
public:
    static GameplaySys& get();
    GameplaySys() = default;
    void start();

private:
    GameplaySys(const GameplaySys&) = delete;
    GameplaySys& operator=(const GameplaySys&) = delete;
    GameplaySys(GameplaySys&&) = delete;
    GameplaySys& operator=(GameplaySys&&) = delete;
};
