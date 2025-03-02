#include "GameplaySys.hpp"

#include "Constants.hpp"
#include "Entities/CameraEntity.hpp"
#include "Entities/PlayerEntity.hpp"
#include "ResourceSys/ResourceSys.hpp"

// Static
GameplaySys& GameplaySys::get() {
    static std::unique_ptr<GameplaySys> instance = std::make_unique<GameplaySys>();
    return *instance;
}

void GameplaySys::start() {
    // Create camera
    CameraEntity camera;
    CameraInfoComp& info = camera.get<CameraInfoComp>();
    PositionComp& cameraPos = camera.get<PositionComp>();
    info.horizFOV = Constants::HORIZ_FOV;
    info.aspectRatio = static_cast<float>(Constants::SIZE_X) / Constants::SIZE_Y;
    info.nearClippingPlane = 2.0f;
    info.farClippingPlane = 10000.0f;
    info.upVector = {0.0f, 1.0f, 0.0f};
    info.direction = {10, 0, 10, 1};  //{10, 20, 10, 1};
    cameraPos.coords = {12, 3, 15};   //{20, 50, 35};

    CameraEntity::instances.emplace_back(camera);

    // Create player
    PlayerEntity player;
    auto [position, renderable, motion, frictionComp] = player.getComponents();
    position.coords.x = 10;
    position.coords.y = 0;
    position.coords.z = 10;
    renderable.objectResource = ResourceSys::get().getObjResource("low_poly_blendered");
    renderable.setMeshes(renderable.objectResource->objMeshes);
    renderable.shader = ResourceSys::get().getShaderResource("basic_pbr");

    PlayerEntity::instances.emplace_back(player);
}
