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
    CameraLensComp& lens = camera.get<CameraLensComp>();
    lens.horizFOV = Constants::HORIZ_FOV;
    lens.aspectRatio = static_cast<float>(Constants::SIZE_X) / Constants::SIZE_Y;
    lens.nearClippingPlane = 0.0001f;
    lens.farClippingPlane = 10000.0f;
    CameraEntity::instances.emplace_back(camera);

    // Create car
    CarEntity car;
    auto [position, renderable] = car.getComponents();
    position.coords.x = 10;
    position.coords.y = 10;
    position.coords.z = 10;
    renderable.mesh = ResourceSys::get().getObjResource("cow")->objMeshes[0];

    CarEntity::instances.emplace_back(car);
}
