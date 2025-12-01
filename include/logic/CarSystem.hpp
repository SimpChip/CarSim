#ifndef CARSIM_CARSYSTEM_HPP
#define CARSIM_CARSYSTEM_HPP

#include "CarEntity.hpp"
#include "controllers/AiController.hpp"
#include "controllers/PlayerController.hpp"

class CarSystem {
public:
    explicit CarSystem(EventBus& bus) : bus_(bus) {};

    void addPlayerCar(const threepp::Vector3& pos, const float rot) {
        auto logic = std::make_shared<Car>(pos, rot, 8);
        playerCarId_ = logic->getId();
        playerCarIndex_ = cars_.size();
        auto entity = std::make_unique<CarEntity>(std::move(logic), nullptr);
        Car& logicRef = *entity->logic;
        entity->controller = std::make_unique<PlayerController>(bus_,logicRef);
        cars_.emplace_back(std::move(entity));
    }

    void addAICar(const threepp::Vector3& pos, const float rot, const std::vector<RoadSample>& samples, const int startTargetidx, const float skill) {
        auto logic = std::make_shared<Car>(pos, rot, 8);
        auto entity = std::make_unique<CarEntity>(std::move(logic), nullptr);
        entity->controller = std::make_unique<AIController>(samples, pos, skill);
        cars_.emplace_back(std::move(entity));
    }

    [[nodiscard]] CarEntity& getPlayerCar() const {
        return *cars_[playerCarIndex_];
    }
    [[nodiscard]] std::vector<std::shared_ptr<CarEntity>> getCars() const {
        return cars_;
    }
    [[nodiscard]] int getPlayerCarID() const {
        return playerCarId_;
    }

    [[nodiscard]] std::vector<CarData> snapshot() const {
        std::vector<CarData> data;
        data.reserve(cars_.size());
        for (const auto& car : cars_) {
            data.push_back(car->logic->snapshot());
        }
        return data;
    }
private:
    EventBus& bus_;
    std::vector<std::shared_ptr<CarEntity>> cars_;
    int playerCarIndex_ = 0;
    int playerCarId_ = 0;
};

#endif //CARSIM_CARSYSTEM_HPP