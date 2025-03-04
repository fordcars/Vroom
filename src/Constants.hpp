#pragma once

#include <math.h>

#include <glm/glm.hpp>

namespace Constants {
constexpr const char* GAME_NAME = "Vroom!";
constexpr const char* RESOURCE_DIR = "rsrc";

constexpr unsigned OPENGL_MAJOR_VERSION = 4;
constexpr unsigned OPENGL_MINOR_VERSION = 3;
constexpr unsigned SIZE_X = 1024;
constexpr unsigned SIZE_Y = 800;
constexpr unsigned TARGET_UPDATE_FREQ = 120; // Steps per second
constexpr unsigned AA_SAMPLES = 8;
const float HORIZ_FOV = glm::radians(90.0f); // In radians

constexpr glm::vec3 BG_COLOR(0.78f, 0.78f, 1.0f);

constexpr unsigned MAX_BONES = 500;
} // namespace Constants
