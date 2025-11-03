//
// Created by Mella on 18.10.2025.
//



#include <map>

#include "cmake-build-debug-mingw/_deps/threepp-src/src/threepp/renderers/gl/GLProperties.hpp"
#include <threepp/threepp.hpp>
#include <iostream>
#include <regex>
#include "CarVisuals.hpp"
#include "MapVisuals.hpp"

class GameScene
{
public:
    std::shared_ptr<threepp::Scene> scene_;
    std::unique_ptr<CarVisuals> carVisuals_;
    std::unique_ptr<MapVisuals> mapVisuals_;
    std::shared_ptr<threepp::PerspectiveCamera> camera;
    std::shared_ptr<threepp::Mesh> colBox;

    float scale_ = 10.f;
    float cameraUpdateInput = 0.f;
    float cameraDistance_ = 5.f;
    float cameraHeight_ = 2.f;

    GameScene(std::string mapName, MapLogic& mapLogic) {
        scene_ = threepp::Scene::create();

        carVisuals_ = std::make_unique<CarVisuals>(scale_, "cooltruck");
        scene_->add(carVisuals_->getCarMesh());

        mapVisuals_ = std::make_unique<MapVisuals>(mapName, mapLogic);
        scene_->add(mapVisuals_->getMapMesh());

        scene_->fog = threepp::Fog(threepp::Color::whitesmoke, 1500, 3000);

        camera = threepp::PerspectiveCamera::create(80, 16.0f/9.0f, 10.f, 4000);
        auto grid = threepp::GridHelper::create(1000, 10, threepp::Color::yellowgreen);
        //scene_->add(grid);

        auto axis = threepp::AxesHelper::create(100);
        scene_->add(axis);

        auto light = threepp::DirectionalLight::create(threepp::Color::white, 0.5f);
        light->position.set(100, 100, 200);
        scene_->add(light);

        auto ambient = threepp::AmbientLight::create(threepp::Color::white, 0.8f);
        scene_->add(ambient);
    }
    void setCameraDistanceInput(float amount) { cameraUpdateInput = std::clamp(amount, -1.0f, 1.0f); }
    void updateCameraDistance(float dt) {
        cameraDistance_+= 10 * cameraUpdateInput * dt ;
        cameraHeight_+= 5 * cameraUpdateInput * dt;
    }

    void update(CarLogic& carLogic ,float& dt)
    {
        updateCameraDistance(dt);
        carVisuals_->update(carLogic, dt);

        float camDistance = cameraDistance_ * scale_ + 3 * scale_;
        float camHeight = cameraHeight_ * scale_ + 2 * scale_;

        const auto forwardDir = carLogic.forwardVector();
        const threepp::Vector3 camOffset = forwardDir * camDistance;
        threepp::Vector3 camPos = carLogic.getPosition() - camOffset;
        camPos.z = camHeight;

        float t = 1.0f - std::pow(1.0f - 0.1f, dt * 60.0f);
        camera->position.lerp(camPos, t );
        camera->up = threepp::Vector3(0, 0, 1);
        camera->lookAt(carLogic.getPosition());
    }
};