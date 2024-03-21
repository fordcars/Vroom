#include "GameSys.hpp"

#include "Entities/CameraEntity.hpp"
#include "Entities/CarEntity.hpp"
#include "Constants.hpp"
#include "ResourceSys/ResourceSys.hpp"

// Static
GameSys& GameSys::get() {
    static std::unique_ptr<GameSys> instance = std::make_unique<GameSys>();
    return *instance;
}

void GameSys::start() {
    // Create camera
    CameraEntity camera;
    CameraInfoComp& info = camera.get<CameraInfoComp>();
    PositionComp& cameraPos = camera.get<PositionComp>();
    info.horizFOV = Constants::HORIZ_FOV;
    info.aspectRatio = static_cast<float>(Constants::SIZE_X) / Constants::SIZE_Y;
    info.nearClippingPlane = 0.0001f;
    info.farClippingPlane = 10000.0f;
    info.upVector = {0.0f, 1.0f, 0.0f};
    info.direction = {10, 10, 10, 1};
    cameraPos.coords = {10, 10, 20};

    CameraEntity::instances.emplace_back(camera);

    // Create car
    CarEntity car;
    auto [position, renderable] = car.getComponents();
    position.coords.x = 10;
    position.coords.y = 10;
    position.coords.z = 10;
    renderable.mesh = ResourceSys::get().getObjResource("cow")->objMeshes[0];
    renderable.shader = ResourceSys::get().getShaderResource("basic");

    CarEntity::instances.emplace_back(car);
}
