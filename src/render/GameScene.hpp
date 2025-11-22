

#ifndef CARSIM_GAMESCENE_HPP
#define CARSIM_GAMESCENE_HPP

#include <map>
#include <vector>
#include <memory>
#include <threepp/threepp.hpp>

#include "CarVisuals.hpp"
#include "MapVisuals.hpp"
#include "PickupVisuals.hpp"
#include "ResourceManager.hpp"
#include "GameLogic.hpp"

class GameScene {
public:


    std::shared_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::PerspectiveCamera> camera;

    std::vector<std::shared_ptr<CarVisuals>> carVisuals_;
    std::unique_ptr<MapVisuals> mapVisuals_;
    std::unique_ptr<PickupVisuals> pickupVisuals_;

    GameScene(
        const std::string& mapName,
        const std::string& modelName,
        float scale,
        const std::vector<std::unique_ptr<CarEntity>>& cars,
        const MapLogic& mapLogic,
        const CheckpointLogic& cpLogic,
        const PickupLogic& pickupLogic,
        const threepp::Vector3& carStartPos,
        ResourceManager& resources
    )
    : scene_(threepp::Scene::create()),
      mapVisuals_(std::make_unique<MapVisuals>(mapName, mapLogic, cpLogic)),
      pickupVisuals_(std::make_unique<PickupVisuals>(pickupLogic.getPickups(), scale)),
      scale_(scale)
    {
        carVisuals_.reserve(cars.size());
        std::cout << cars.size() << std::endl;
        const std::vector<std::string> colors = {"blue", "green", "yellow", "pink", "white", "purple", "orange", "golden"};
        for (size_t i = 0; i < cars.size(); i++) {
            std::string name = "redcar";

            std::string color = colors[i % colors.size()];
            if (i == 0) {
                name = modelName;
                color = "basic";
            }
            const auto& logic = cars[i]->logic;

            auto vis = std::make_shared<CarVisuals>(
                name,
                logic.getPosition(),
                logic.getRotation(),
                scale_,
                resources,
                color
            );

            carVisuals_.emplace_back(vis);
            scene_->add(vis->getCarMesh());
        }

        scene_->add(mapVisuals_->getMapMesh());

        pickupVisuals_->addToScene(scene_);

        scene_->fog = threepp::Fog(threepp::Color::whitesmoke, 1500, 4000);

        const auto ambient = threepp::AmbientLight::create(threepp::Color::white, 0.8f);
        scene_->add(ambient);

        const auto light = threepp::DirectionalLight::create(threepp::Color::white, 0.5f);
        light->position.set(100, 100, 200);
        scene_->add(light);

        camera = threepp::PerspectiveCamera::create(80, 16.f/9.f, 10.f, 5000.f);
        camera->position = carStartPos;
        camera->lookAt(carStartPos);

        camera->up = threepp::Vector3(0, 0, 1);
    }

    void setCameraDistanceInput(const float amount) {
        cameraUpdateInput = std::clamp(amount, -1.f, 1.f);
    }

    void updateCameraDistance(const float dt) {
        cameraDistance_ += 10 * cameraUpdateInput * dt;
        cameraHeight_   += 5 * cameraUpdateInput * dt;

        cameraDistance_ = std::clamp(cameraDistance_, 2.f, 20.f);
        cameraHeight_   = std::clamp(cameraHeight_,   1.f, 10.f);
    }

    void update(const std::vector<std::unique_ptr<CarEntity>>& cars, const PickupLogic& pickupLogic, float dt) const {

        const size_t count = std::min(cars.size(), carVisuals_.size());

        for (size_t i = 0; i < count; i++) {

            const auto& logic = cars[i]->logic;

            const auto pos    = logic.getPosition();
            const auto rot    = logic.getRotation();
            const float speed = logic.getVelocity().length();
            const float steer = logic.getCurrentSteeringInput();
            carVisuals_[i]->update(pos, rot, speed, steer, dt);

            if (i == 0) {
                updateCameraFollow(logic, dt);
            }
        }
        pickupVisuals_->update(pickupLogic.getPickups(), dt);
    }

private:
    float scale_ = 1.0f;
    float cameraUpdateInput = 0.f;
    float cameraDistance_   = 5.f;
    float cameraHeight_     = 4.f;

    void updateCameraFollow(const CarLogic& logic, const float dt) const {
        const auto pos = logic.getPosition();
        const threepp::Vector3 forward = logic.forwardVector().normalize();

        const float dist  = (cameraDistance_+ (0.5f * logic.getBoostFactor()) ) * scale_;
        const float height = cameraHeight_ * scale_;

        threepp::Vector3 desired = pos - forward * dist;
        desired.z += height;

        const float smooth = 1.f - std::pow(1.f - 0.2f, dt * 60.f);
        camera->position.lerp(desired, smooth);

        camera->lookAt(pos);
    }
};

#endif // CARSIM_GAMESCENE_HPP
