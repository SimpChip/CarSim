//
// Created by Mella on 13.11.2025.
//

#ifndef CARSIM_GAMELOGIC_HPP
#define CARSIM_GAMELOGIC_HPP

#include "controller/IControler.hpp"
#include "MapLogic.hpp"
#include "CarLogic.hpp"
#include "CheckpointLogic.hpp"
#include "PickupLogic.hpp"

#include "random"
#include "types/data.hpp"
#include "data/CarEntity.hpp"

class GameLogic {
public:
    explicit GameLogic(const std::string &mapName, const int aiCount)
        : mapLogic_(mapName)
        , checkpointLogic_(mapLogic_.road.getSamples(), mapLogic_.road.getWidth(), 0)
        , pickupLogic_(mapLogic_.road.getSamples(), mapLogic_.road.getWidth())
    {
        const int startIdx = checkpointLogic_.getRoadSampleStartIndex();
        auto [playerStart, playerRot, playerStartIdx] = mapLogic_.getStartPositionAndRotation(aiCount+1, startIdx, 4);
        addPlayerCar(playerStart, playerRot);
        for (size_t i = 0; i < aiCount; i++) {
            auto [carStartPos, carStartRot, carStartIdx] = mapLogic_.getStartPositionAndRotation(i, startIdx, 4);
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<float> distrib(0.3f, 1.0f);
            const float randomSkill = distrib(gen);
            addAICar(carStartPos, carStartRot, carStartIdx, randomSkill);
        }
    }


    void update(const float dt) {
        handleLapTimer(dt);

        collisionManager.clear();

        for (auto& pickup : pickupLogic_.getPickups()) {
            pickup.update(dt);
            if (pickup.isActive()) {
                collisionManager.registerCollidable(&pickup);
            }
        }
        for (const auto& car : cars_) {
            car->update(dt);
            collisionManager.registerCollidable(car->logic.get());
        }
        const auto collisions = collisionManager.detectCollisions();
        CollisionManager::resolveCollisions(collisions);
    }

    void addPlayerCar(const Vector3& pos, const float rot) {
        auto logic = std::make_unique<CarLogic>(pos, rot, 8);
        auto entity = std::make_unique<CarEntity>(std::move(logic), nullptr);
        CarLogic& logicRef = *entity->logic;
        entity->controller = std::make_unique<PlayerController>(logicRef);
        cars_.emplace_back(std::move(entity));
    }

    CarEntity& getPlayerCar() const {
        return *cars_[0];
    }
    void addAICar(const Vector3& pos, const float rot, const int startTargetidx, const float skill) {
        auto logic = std::make_unique<CarLogic>(pos, rot, 8);
        auto entity = std::make_unique<CarEntity>(std::move(logic), nullptr);
        entity->controller = std::make_unique<AIController>(mapLogic_.road.getSamples(), startTargetidx , skill);
        cars_.emplace_back(std::move(entity));
    }

    GameState snapshot() const {
        GameState state;
        state.scale = globScale;
        for (auto& car : cars_) state.cars.push_back(car->logic->snapshot());
        state.mapData = mapLogic_.snapshot();
        state.pickups = pickupLogic_.snapshot();
        state.checkpointsData = checkpointLogic_.snapshot();
        state.lapTimer = lapTimer_;
        state.lastLapTime = lastLapTime_;
        return state;
    }
private:

    float globScale = 8;
    MapLogic mapLogic_;
    CheckpointLogic checkpointLogic_;
    PickupLogic pickupLogic_;
    std::vector<std::unique_ptr<CarEntity>> cars_;
    CollisionManager collisionManager;
    float lapTimer_ = 0.f;
    float lastLapTime_ = 0.f;
    float lapCompleteTimer_ = 0.f;

    void handleLapTimer(const float dt) {
        lapTimer_ += dt;
        if (checkpointLogic_.allCheckpointsPassed()) {
            lastLapTime_ = lapTimer_;
            lapTimer_ = 0.f;
        }
    }
};


#endif //CARSIM_GAMELOGIC_HPP