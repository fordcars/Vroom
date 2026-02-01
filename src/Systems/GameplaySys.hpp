#pragma once

class GameplaySys {
public:
    static GameplaySys& get();

    GameplaySys() = default;
    GameplaySys(const GameplaySys&) = delete;
    GameplaySys& operator=(const GameplaySys&) = delete;
    GameplaySys(GameplaySys&&) = delete;
    GameplaySys& operator=(GameplaySys&&) = delete;

    void start();
    void update(float deltaTime);
};
