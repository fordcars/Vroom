#pragma once
#include <filesystem>
#include <unordered_map>
#include <string>

class GameSys {
public:
    static GameSys& get();
    void start();
};
