//
// Created by Mella on 26.11.2025.
//

#ifndef CARSIM_CARENTITY_HPP
#define CARSIM_CARENTITY_HPP

#include "CarLogic.hpp"
#include "memory"
#include "controllers/IController.hpp"

struct CarEntity {
    std::shared_ptr<CarLogic> logic;
    std::unique_ptr<IController> controller;
    CarEntity(std::shared_ptr<CarLogic> logicIn, std::unique_ptr<IController> ctrlIn)
        : logic(std::move(logicIn)), controller(std::move(ctrlIn)) {}

    void update(const float dt) const {
        if (controller) controller->update(dt, *logic);
        logic->update(dt);
    }
};

#endif //CARSIM_CARENTITY_HPP