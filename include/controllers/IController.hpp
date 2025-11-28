//
// Created by Mella on 27.11.2025.
//

#ifndef CARSIM_ICONTROLLER_HPP
#define CARSIM_ICONTROLLER_HPP

#include "logic/CarLogic.hpp"

class IController {
public:
    virtual ~IController() = default;
    virtual void update(float dt, CarLogic& car) = 0;
};

#endif //CARSIM_ICONTROLLER_HPP