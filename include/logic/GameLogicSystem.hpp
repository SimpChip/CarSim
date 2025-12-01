//
// Created by Mella on 13.11.2025.
//

#ifndef CARSIM_GAMELOGIC_HPP
#define CARSIM_GAMELOGIC_HPP

#include <memory>
#include "random"
#include "collision/CollisonManager.hpp"
#include "MapSystem.hpp"
#include "LapSystem.hpp"
#include "PickupSystem.hpp"
#include "CarSystem.hpp"
#include "TrackGenerator.hpp"
#include "types/data.hpp"
#include "types/SnapShots.hpp"

class GameLogicSystem {
public:
    explicit GameLogicSystem(EventBus& bus, const ResourceManager& rm, const std::string &mapName, const int aiCount, const int startIdx)
    : carSystem_(bus),
     mapManager_(mapName, rm),
        track_(mapManager_),
      lapManager_(bus, startIdx),
      collisionManager_(bus), startIdx_(startIdx)
    {
        pickupManager_.setPickups(track_.generatePickups());
        auto [checkpoints, startCheckpointIndex] = track_.generateCheckpoints(startIdx, 22);
        lapManager_.setCheckpoints(checkpoints);
        const auto& startGrid = track_.generateStartGrid(aiCount+1, 4, startCheckpointIndex );
        const auto& roadSamples = track_.getRoadSamples();

        carSystem_.addPlayerCar(startGrid[startGrid.size()-1].first, startGrid[startGrid.size()-1].second);
        for (int i = 0; i < aiCount; ++i) {
            carSystem_.addAICar(startGrid[i].first, startGrid[i].second, roadSamples, 0, 0.9);
        }

        for (auto& cp : lapManager_.getCheckpoints()) {
            collisionManager_.registerStatic(cp);
        }
        for (auto& pickup : pickupManager_.getPickups()) {
            collisionManager_.registerDynamic(pickup);
        }
        for (const auto& car : carSystem_.getCars()) {
            collisionManager_.registerCar(car->logic);
            lapManager_.registerCar(car->logic->getId());
        }

    }

    void update(const float dt) {
        lapManager_.update(dt);
        if (!lapManager_.hasStarted()) {lapManager_.countDown(dt); return;}

        auto& pickups = pickupManager_.getPickups();
        for (const auto & pickup : pickups) {
            pickup->update(dt);
            if (pickup->isActive()) {
                collisionManager_.updateDynamicPosition(pickup);
            }
        }
        for (const auto& car : carSystem_.getCars()) {
            car->update(dt);
            collisionManager_.updateCarPosition(car->logic);
        }
        collisionManager_.detectCollisions(dt);
    }

    GameSnapShot snapshot() {
        GameSnapShot state;
        state.scale = globScale_;
        state.startIdx = startIdx_;
        state.cars = carSystem_.snapshot();
        state.mapData = mapManager_.snapshot();
        state.pickups = pickupManager_.snapshot();
        state.playerCarId = carSystem_.getPlayerCarID();
        state.lapManagerData = lapManager_.snapshot();
        return state;
    }


private:
    float globScale_ = 8;
    float startIdx_ = 0;
    CarSystem carSystem_;
    MapSystem mapManager_;
    TrackGenerator track_;
    PickupSystem pickupManager_;
    LapSystem lapManager_;
    CollisionManager collisionManager_;
};


#endif //CARSIM_GAMELOGIC_HPP