#pragma once

#include <math.h>

#include <glm/glm.hpp>

#include "Utils/StringIndexor.hpp"

namespace Constants {
constexpr const char* GAME_NAME = "Vroom!";
constexpr const char* RESOURCE_DIR = "rsrc";

constexpr unsigned OPENGL_MAJOR_VERSION = 4;
constexpr unsigned OPENGL_MINOR_VERSION = 3;
constexpr unsigned DEFAULT_WINDOW_SIZE_X = 1024;
constexpr unsigned DEFAULT_WINDOW_SIZE_Y = 800;
constexpr unsigned NO_VSYNC_MAX_FPS = 120; // Max FPS if VSync is off
const bool ENABLE_VSYNC = true;
constexpr unsigned AA_SAMPLES = 8;
const float HORIZ_FOV = glm::radians(90.0f); // In radians

constexpr unsigned MAX_BONES_PER_SKINNED_MESH = 500;

// Strings used as map keys, but known at compile time
// are defined here; prefer to use a simple array with
// these values instead of a map of std::strings.
using UniformName = Utils::StringIndexor<
    "MVP", "modelMatrix", "viewMatrix", "projectionMatrix", "normalMatrix", "time",
    "isSkinned", "lightPos_worldspace", "lightDiffuseColor", "lightSpecularColor",
    "lightIntensity", "positionTex", "normalTex", "albedoTex", "metallicTex",
    "roughnessTex", "baseColorTex", "hasBaseColorTex", "hasNormalTex",
    "metallicRoughnessTex", "hasMetallicRoughnessTex", "emissiveTex", "hasEmissiveTex",
    "normalScale">;
using UniformBlockName = Utils::StringIndexor<"ObjMaterialsBlock", "SkinTransformBlock">;
using AnimationName = Utils::StringIndexor<"Normal Walk", "Zombie Walk", "Happy">;

} // namespace Constants
