//
// Created by Mella on 13.11.2025.
//

#ifndef CARSIM_GAMELOGIC_HPP
#define CARSIM_GAMELOGIC_HPP

#include <memory>

#include "controllers/PlayerController.hpp"
#include "controllers/AiController.hpp"
#include "CarLogic.hpp"
#include "MapLogic.hpp"
#include "LapManager.hpp"
#include "PickupLogic.hpp"
#include "collision/CollisonManager.hpp"

#include "random"
#include "types/data.hpp"
#include "CarEntity.hpp"

class GameLogic {
public:
    explicit GameLogic(const std::string &mapName, const int aiCount)
        : mapLogic_(mapName)
        , pickupLogic_(mapLogic_.road.getSamples(), mapLogic_.road.getWidth())
        , lapManager_(mapLogic_.road.getSamples(), mapLogic_.road.getWidth(), 0)
    {
        constexpr int startIdx = 0;
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
        for (auto& cp : lapManager_.getCheckpoints()) {
            collisionManager.registerStatic(cp);
        }
        for (auto& pickup : pickupLogic_.getPickups()) {
            collisionManager.registerDynamic(pickup);
        }
        for (const auto& car : cars_) {
            collisionManager.registerCar(car->logic);
        }
    }
    void preUpdate(const float dt) {
        lapManager_.preUpdate();
    }

    void update(const float dt) {
        lapManager_.update(dt);
        collisionManager.clearDynamic();
        int counter = 0;
        auto& pickups = pickupLogic_.getPickups();
        for (size_t i = 0; i < pickups.size(); i++) {
            pickups[i]->update(dt);
            if (counter == 1) {
             //   std::cout << "Pickup Position :"<< pickups[i]->getPosition2D() << std::endl;
            }
            if (pickups[i]->isActive()) {
                collisionManager.updateDynamicPosition(pickups[i]);
            }
            counter++;
        }
        counter = 0;
        for (const auto& car : cars_) {
            if (counter == 0) {
               // std::cout << "Car position :" << cars_.begin()->get()->logic->getPosition2D() << std::endl;
            }
            car->update(dt);
            collisionManager.updateCarPosition(car->logic);
            counter++;
        }
        collisionManager.detectCollisions();
    }

    void addPlayerCar(const Vector3& pos, const float rot) {
        auto logic = std::make_unique<CarLogic>(pos, rot, 8);
        playerCarId_ = logic->getId();
        lapManager_.registerCar(playerCarId_);
        auto entity = std::make_unique<CarEntity>(std::move(logic), nullptr);
        CarLogic& logicRef = *entity->logic;
        entity->controller = std::make_unique<PlayerController>(logicRef);
        cars_.emplace_back(std::move(entity));
    }

    CarEntity& getPlayerCar() const {
        return *cars_[0];
    }
    void addAICar(const threepp::Vector3& pos, const float rot, const int startTargetidx, const float skill) {
        auto logic = std::make_unique<CarLogic>(pos, rot, 8);
        lapManager_.registerCar(logic->getId());
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
        state.playerCarId = playerCarId_;
        state.lapManagerData = lapManager_.snapshot();
        return state;
    }
private:

    float globScale = 8;
    MapLogic mapLogic_;
    PickupLogic pickupLogic_;
    std::vector<std::shared_ptr<CarEntity>> cars_;
    LapManager lapManager_;
    CollisionManager collisionManager;
    int playerCarId_ = 0;

};


#endif //CARSIM_GAMELOGIC_HPP