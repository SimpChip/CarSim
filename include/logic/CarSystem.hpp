#ifndef CARSIM_CARSYSTEM_HPP
#define CARSIM_CARSYSTEM_HPP

#include "../entities/CarEntity.hpp"
#include "core/EventBus.hpp"
#include "threepp/threepp.hpp"
#include <memory>

#include "types/gameSnapshots.hpp"


class CarSystem {
public:
    explicit CarSystem(EventBus& bus);

    void addPlayerCar(const threepp::Vector3& pos, float rot);

    void addAICar(const threepp::Vector3& pos, float rot, const std::vector<RoadSample>& samples,int startTargetidx,float skill);

    [[nodiscard]] CarEntity& getPlayerCar() const;

    [[nodiscard]] std::vector<std::shared_ptr<CarEntity>> getCars() const;

    [[nodiscard]] int getPlayerCarID() const;

    [[nodiscard]] std::vector<CarSnap> snapshot() const;

private:
    EventBus& bus_;
    std::vector<std::shared_ptr<CarEntity>> cars_;
    int playerCarIndex_ = 0;
    int playerCarId_ = 0;
};

#endif //CARSIM_CARSYSTEM_HPP