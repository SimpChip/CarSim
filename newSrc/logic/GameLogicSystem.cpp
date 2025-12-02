#include "logic/GameLogicSystem.hpp"
#include <random>

GameLogicSystem::GameLogicSystem(
    EventBus& bus,
    const int aiCount,
    const int startIdx,
    const std::shared_ptr<MapConfig>& mapConfig
    ) :
    startIdx_(startIdx),
    carSystem_(bus),
    mapSystem_(mapConfig),
    lapSystem_(bus, startIdx),
    collisionManager_(bus),
    trackGenerator_(mapSystem_)
{
    generateWorld(aiCount, startIdx);
    registerAllSystems();
}

void GameLogicSystem::generateWorld(const int aiCount, const int startIdx) {
    std::cout << "Generating the World" << "\n";
    pickupSystem_.setPickups(trackGenerator_.generatePickups());
    auto [checkpoints, startCheckpointIndex] = trackGenerator_.generateCheckpoints(startIdx, 22);
    lapSystem_.setCheckpoints(checkpoints);
    const auto& startGrid = trackGenerator_.generateStartGrid(aiCount+1, 4, startCheckpointIndex );
    const auto& roadSamples = trackGenerator_.getRoadSamples();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> skillDist(0.6f, 0.9f);

    carSystem_.addPlayerCar(startGrid[startGrid.size()-1].first, startGrid[startGrid.size()-1].second);
    for (int i = 0; i < aiCount; ++i) {
        const float skill = skillDist(gen);
        carSystem_.addAICar(startGrid[i].first, startGrid[i].second, roadSamples, 0, skill);
    }
}

void GameLogicSystem::registerAllSystems() {
    std::cout << "Register All Systems" << "\n";
    for (auto& cp : lapSystem_.getCheckpoints()) {
        collisionManager_.registerStatic(cp);
    }
    for (auto& pickup : pickupSystem_.getPickups()) {
        collisionManager_.registerDynamic(pickup);
    }
    for (const auto& car : carSystem_.getCars()) {
        collisionManager_.registerCar(car->logic);
        lapSystem_.registerCar(car->logic->getId());
    }
}

void GameLogicSystem::update(const float dt) {
    lapSystem_.update(dt);
    if (!lapSystem_.hasStarted()) {lapSystem_.countDown(dt); return;}

    const auto& pickups = pickupSystem_.getPickups();
    for (const auto & pickup : pickups) {
        pickup->update(dt);
        if (pickup->isActive()) {
            collisionManager_.updateDynamicPosition(pickup);
        }
    }
    const auto& cars = carSystem_.getCars();
    for (const auto& car : cars) {
        car->update(dt);
        collisionManager_.updateCarPosition(car->logic);
    }
    collisionManager_.detectCollisions(dt);
}

GameSnap GameLogicSystem::snapshot() {
    GameSnap snap;
    snap.scale = globScale_;
    snap.startIdx = startIdx_;
    snap.cars = carSystem_.snapshot();
    snap.map = mapSystem_.snapshot();
    snap.pickups = pickupSystem_.snapshot();
    snap.playerCarId = carSystem_.getPlayerCarID();
    snap.lap = lapSystem_.snapshot();
    return snap;
}
