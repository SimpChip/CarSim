#ifndef CARSIM_GAMESCENE_HPP
#define CARSIM_GAMESCENE_HPP

#include "threepp/threepp.hpp"
#include "../helpers/ResourceManager.hpp"
#include "visuals/CarVisuals.hpp"
#include "types/data.hpp"
#include <vector>
#include "types/gameSnapshots.hpp"

class GameScene {
public:
    std::shared_ptr<threepp::Scene> scene_;
    GameScene(
        const GameSnap& state,
        ResourceManager& rm,
        const std::string& playerModel
    );

    void update(const GameSnap& state, float dt) const;
    void render(threepp::GLRenderer& renderer) const;

private:
    float scale_ = 1.0f;
    float cameraUpdateInput = 0.f;
    float cameraDistance_   = 5.5f;
    float cameraHeight_     = 4.f;

    std::shared_ptr<threepp::PerspectiveCamera> camera_;

    std::shared_ptr<threepp::Mesh> roadMesh_;
    std::shared_ptr<threepp::Mesh> sandMesh_;
    std::shared_ptr<threepp::Mesh> groundMesh_;

    std::shared_ptr<threepp::Group> pickupGroup_;
    std::shared_ptr<threepp::Group> checkpointGroup_;
    std::vector<std::shared_ptr<threepp::Group>> pickupMeshes_;
    std::vector<std::shared_ptr<CarVisuals>> carVisuals_;

    void setupLighting() const;

    void setupFog() const;

    void setupCamera(const threepp::Vector3& startPos);

    void buildCarVisuals(const std::vector<CarSnap>& cars, const std::string& playerModel, float scale, ResourceManager& rm);

    void buildGroundVisuals(ResourceManager& rm, int width, int height);

    void buildSandShoulder(ResourceManager& rm, const std::vector<RoadSample>& samples, float roadWidth, float shoulderWidth, bool looped);

    void buildRoadMesh(ResourceManager& resources, const std::vector<RoadSample>& samples,float roadWidth, bool looped);

    void buildCheckPointsVisuals(const std::vector<CpSnap>& checkpoints);

    void buildPickupVisuals(ResourceManager& resources, const std::vector<PickupSnap> &pickups, float scale);

    void updateCameraFollow(const CarSnap& car, float scale, float dt) const;
};

#endif // CARSIM_GAMESCENE_HPP
