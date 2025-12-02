#ifndef CARSIM_CARENTITY_HPP
#define CARSIM_CARENTITY_HPP

#include "Car.hpp"
#include "memory"
#include "controllers/IController.hpp"

struct CarEntity {
    std::shared_ptr<Car> logic;
    std::unique_ptr<IController> controller;

    CarEntity(std::shared_ptr<Car> logicIn, std::unique_ptr<IController> ctrlIn)
        : logic(std::move(logicIn)), controller(std::move(ctrlIn)) {}

    void update(const float dt) const {
        if (controller) controller->update(dt, *logic);
        logic->update(dt);
    }
};

#endif //CARSIM_CARENTITY_HPP