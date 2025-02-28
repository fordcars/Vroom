#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>

class GameSys {
public:
    static GameSys& get();
    GameSys() = default;
    void start();

private:
    GameSys(const GameSys&) = delete;
    GameSys& operator=(const GameSys&) = delete;
    GameSys(GameSys&&) = delete;
    GameSys& operator=(GameSys&&) = delete;
};
