#pragma once
#include <SDL2/SDL.h>
#include "System.hpp"

class ResourceSys : protected System<> {
public:
    void loadResources();
};
