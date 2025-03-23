#include "GameplaySys.hpp"

#include "Constants.hpp"
#include "Entities/CameraEntity.hpp"
#include "Entities/LightEntity.hpp"
#include "Entities/PlayerEntity.hpp"
#include "Entities/PropEntity.hpp"
#include "Entities/SkyboxEntity.hpp"
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
    info.aspectRatio = static_cast<float>(Constants::DEFAULT_WINDOW_SIZE_X) / Constants::DEFAULT_WINDOW_SIZE_Y;
    info.nearClippingPlane = 2.0f;
    info.farClippingPlane = 10000.0f;
    info.upVector = {0.0f, 1.0f, 0.0f};
    info.direction = {10, 0, 10, 1};
    cameraPos.coords = {12, 10, 25};

    CameraEntity::instances.emplace_back(std::move(camera));

    // Create player
    {
        PlayerEntity player;
        auto [position, renderable, animationComp, physics, frictionComp, gravityComp] =
            player.getComponents();
        position.coords.x = -5;
        position.coords.y = 3;
        position.coords.z = 10;
        physics.positionOffset = {0, 0.7, 0};
        renderable.objectResource = ResourceSys::get().getObjResource("skelly");
        renderable.shader = ResourceSys::get().getShaderResource("deferred_pbr_skinned");
        animationComp.setAnimation(Constants::AnimationName::get<"Happy">());
        animationComp.startTime =
            0.09; // The walk animation isn't a perfect loop, this helps

        PlayerEntity::instances.emplace_back(std::move(player));
    }

    // Create car
    {
        PropEntity prop;
        auto [position, renderable, frictionComp, physics] = prop.getComponents();
        position.coords.x = 10;
        position.coords.y = 0;
        position.coords.z = 10;
        renderable.objectResource =
            ResourceSys::get().getObjResource("low_poly_blendered");
        renderable.shader = ResourceSys::get().getShaderResource("deferred_pbr");
        PropEntity::instances.emplace_back(std::move(prop));
    }

    // Create car2
    {
        PropEntity prop;
        auto [position, renderable, frictionComp, physics] = prop.getComponents();
        position.coords.x = 17;
        position.coords.y = 0;
        position.coords.z = 10;
        position.rotation.y = 1.6f;
        renderable.objectResource =
            ResourceSys::get().getObjResource("low_poly_blendered");
        renderable.shader = ResourceSys::get().getShaderResource("deferred_pbr");
        PropEntity::instances.emplace_back(std::move(prop));
    }

    // Create car3
    {
        PropEntity prop;
        auto [position, renderable, frictionComp, physics] = prop.getComponents();
        position.coords.x = 30;
        position.coords.y = 2;
        position.coords.z = -20;
        position.scale = glm::vec3(2.0f);
        position.rotation.y = 1.0f;
        renderable.objectResource =
            ResourceSys::get().getObjResource("low_poly_blendered");
        renderable.shader = ResourceSys::get().getShaderResource("deferred_pbr");
        PropEntity::instances.emplace_back(std::move(prop));
    }

    // Create floor
    {
        PropEntity prop;
        auto [position, renderable, frictionComp, physics] = prop.getComponents();
        position.coords.x = 0;
        position.coords.y = -1;
        position.coords.z = 0;
        position.scale = {100, 1, 100};
        renderable.objectResource =
            ResourceSys::get().getObjResource("low_poly_blendered");
        renderable.shader = ResourceSys::get().getShaderResource("deferred_pbr");
        PropEntity::instances.emplace_back(std::move(prop));
    }

    // Create light
    {
        LightEntity mainLight;
        auto [position, light, physics] = mainLight.getComponents();
        position.coords.x = 10;
        position.coords.y = 15;
        position.coords.z = 15;
        light.shader = ResourceSys::get().getShaderResource("light_pbr");
        light.intensity = 8.0f;
        LightEntity::instances.emplace_back(std::move(mainLight));
    }

    // Create skybox
    {
        SkyboxEntity sky;
        auto [position, renderable] = sky.getComponents();
        position.scale = {1000, 1000, 1000};
        renderable.objectResource =
            ResourceSys::get().getObjResource("skybox");
        renderable.shader = ResourceSys::get().getShaderResource("flat");
        renderable.shadingType = RenderableComp::ShadingType::ForwardShaded;
        SkyboxEntity::instances.emplace_back(std::move(sky));
    }

    ResourceSys::get().getAudioResource("texasradiofish")->run<ma_sound_start>();
}

void GameplaySys::update(float deltaTime) {
    auto& playerPosition = PlayerEntity::instances[0].get<PositionComp>().coords;
    auto& cameraPosition = CameraEntity::instances[0].get<PositionComp>().coords;
    CameraEntity::instances[0].get<CameraInfoComp>().direction =
        glm::vec4(playerPosition, 1);

    // Update skybox
    SkyboxEntity::instances[0].get<PositionComp>().coords = cameraPosition;

    // Random debug stuff
    PropEntity::instances[1].get<PositionComp>().coords.x = 10 + 2*sin(SDL_GetTicks() / 1000.0f);
    PropEntity::instances[2].get<PositionComp>().rotation.x = SDL_GetTicks() / 1000.0f;

    // PropEntity::instances[0].get<PositionComp>().coords.x = playerPosition.x;
    // PropEntity::instances[0].get<PositionComp>().coords.y = playerPosition.y;
    // PropEntity::instances[0].get<PositionComp>().coords.z = playerPosition.z;

//     // Multicolor light
//     float time = static_cast<float>(SDL_GetTicks()) / 1000.0f;
//     glm::vec3 lightColor;
//     lightColor.r = (sin(time * 2.0f) + 1.0f) / 2.0f;
//     lightColor.g = (sin(time * 0.7f) + 1.0f) / 2.0f;
//     lightColor.b = (sin(time * 1.3f) + 1.0f) / 2.0f;
//     LightEntity::instances[0].get<LightComp>().diffuse = lightColor;
//     LightEntity::instances[0].get<LightComp>().specular = lightColor;
}