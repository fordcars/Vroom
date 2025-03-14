#pragma once

class UISys {
public:
    static UISys& get();
    UISys() = default;

    bool init();
    void update(float deltaTime);

private:
    UISys(const UISys&) = delete;
    UISys& operator=(const UISys&) = delete;
    UISys(UISys&&) = delete;
    UISys& operator=(UISys&&) = delete;
};
