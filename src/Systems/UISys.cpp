#include "UISys.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <memory>

// Static
UISys& UISys::get() {
    static std::unique_ptr<UISys> instance = std::make_unique<UISys>();
    return *instance;
}

bool UISys::init() {
    return true;
}

void UISys::update(float deltaTime) {}
