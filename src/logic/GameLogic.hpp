//
// Created by Mella on 13.11.2025.
//

#ifndef CARSIM_GAMELOGIC_HPP
#define CARSIM_GAMELOGIC_HPP

#include <controller/IControler.hpp>
#include <MapLogic.hpp>
#include <CarLogic.hpp>

#include "CheckpointLogic.hpp"
#include "PickupLogic.hpp"

#include "random"
#include "types/data.hpp"

struct CarEntity {
    CarLogic logic;
    std::unique_ptr<IController> controller;

    CarEntity(CarLogic&& l, std::unique_ptr<IController> c)
    : logic(std::move(l)), controller(std::move(c)) {}

    void update(const float dt) {
        if (controller) controller->update(dt, logic);
        logic.update(dt);
    }
};


class GameLogic {
public:
    float globScale = 8;
    MapLogic mapLogic_;
    CheckpointLogic checkpointLogic_;
    PickupLogic pickupLogic_;
    std::vector<std::unique_ptr<CarEntity>> cars_;
    float lapTimer_ = 0.f;
    float lastLapTime_ = 0.f;
    float lapCompleteTimer_ = 0.f;

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
    void handleLapTimer(const float dt) {
        lapTimer_ += dt;
        if (checkpointLogic_.lapCompleted()) {
            lastLapTime_ = lapTimer_;
            lapTimer_ = 0.f;
        }
    }

    void update(const float dt) {
        handleLapTimer(dt);
        pickupLogic_.update(dt);

        const size_t numCars = cars_.size();
        for (size_t i = 0; i < numCars; ++i) {
            const auto& carA = cars_[i];
            carA->update(dt);
            if (i == 0) {
                checkpointLogic_.handleCheckpointPassing(carA->logic.getPosition(), carA->logic.getPreviousPosition());
            }
            mapLogic_.handleMapCollisions(carA->logic, dt);
            pickupLogic_.handlePickupCollisions(carA->logic, dt);
            for (size_t j = i + 1; j < numCars; ++j) {
                const auto& carB = cars_[j];
                const OBB obbA = carA->logic.getOrientedBoundingBox();
                const OBB obbB = carB->logic.getOrientedBoundingBox();

                if (obbA.intersects(obbB)) {
                    resolveCollisionImpulse(carA->logic, carB->logic);
                }
            }
        }

    }

    GameState snapshot() const {
        GameState state;
        state.scale = globScale;
        for (auto& car : cars_) state.cars.push_back(car->logic.snapshot());
        state.mapData = mapLogic_.snapshot();
        state.pickups = pickupLogic_.snapshot();
        state.checkpointsData = checkpointLogic_.snapshot();
        state.lapTimer = lapTimer_;
        state.lastLapTime = lastLapTime_;
        return state;
    }

    static void resolveCollisionImpulse(CarLogic& carA, CarLogic& carB) {
        Vector3 normal = carA.getPosition() - carB.getPosition();
        normal.normalize();
        const Vector3 relativeVelocity = carA.velocity_ - carB.velocity_;

        const float velocityAlongNormal = relativeVelocity.dot(normal);

        if (velocityAlongNormal > 0) return;

        constexpr float e = 0.1f;
        const float J = -(1.0f + e) * velocityAlongNormal;

        const Vector3 impulse = normal * J;

        carA.applyVelocity(impulse);

        carB.applyVelocity((Vector3(0,0,0) - impulse)) ;
    }

    void addPlayerCar(const Vector3& pos, const float rot) {
        auto entity = std::make_unique<CarEntity>(CarLogic(pos, rot, globScale), nullptr);
        CarLogic& logicRef = entity->logic;
        entity->controller = std::make_unique<PlayerController>(logicRef);
        cars_.emplace_back(std::move(entity));
    }

    CarEntity& getPlayerCar() const {
        return *cars_[0];
    }

    void addAICar(const Vector3& pos, const float rot, const int startTargetidx, const float skill) {
        auto entity = std::make_unique<CarEntity>(CarLogic(pos, rot, globScale), nullptr);
        CarLogic& logicRef = entity->logic;
        entity->controller = std::make_unique<AIController>(mapLogic_.road.getSamples(), startTargetidx , skill);
        cars_.emplace_back(std::move(entity));
    }


};




#endif //CARSIM_GAMELOGIC_HPP