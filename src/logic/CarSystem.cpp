#include "logic/CarSystem.hpp"
#include "controllers/AiController.hpp"
#include "controllers/PlayerController.hpp"
#include "types/gameSnapshots.hpp"

CarSystem::CarSystem(EventBus &bus) : bus_(bus) {}

void CarSystem::addPlayerCar(const threepp::Vector3& pos, const float rot) {
    auto logic = std::make_shared<Car>(pos, rot);
    playerCarId_ = logic->getId();
    playerCarIndex_ = cars_.size();
    auto entity = std::make_unique<CarEntity>(std::move(logic), nullptr);
    Car& logicRef = *entity->logic;
    entity->controller = std::make_unique<PlayerController>(bus_,logicRef);
    cars_.emplace_back(std::move(entity));
}

CarEntity& CarSystem::getPlayerCar() const {
    return *cars_[playerCarIndex_];
}

void CarSystem::addAICar(const threepp::Vector3& pos, const float rot,const std::vector<RoadSample>& samples,int startTargetidx,float skill) {
    auto logic = std::make_shared<Car>(pos, rot);
    auto entity = std::make_unique<CarEntity>(std::move(logic), nullptr);
    entity->controller = std::make_unique<AIController>(samples, pos, skill);
    cars_.emplace_back(std::move(entity));
}

std::vector<std::shared_ptr<CarEntity>> CarSystem::getCars() const {
    return cars_;
}

int CarSystem::getPlayerCarID() const {
    return playerCarId_;
}

std::vector<CarSnap> CarSystem::snapshot() const {
    std::vector<CarSnap> data;
    data.reserve(cars_.size());
    for (const auto& car : cars_) {
        data.push_back(car->logic->snapshot());
    }
    return data;
}