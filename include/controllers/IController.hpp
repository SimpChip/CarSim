//
// Created by Mella on 27.11.2025.
//

#ifndef CARSIM_ICONTROLLER_HPP
#define CARSIM_ICONTROLLER_HPP

#include "logic/Car.hpp"

class IController {
public:
    virtual ~IController() = default;
    virtual void update(float dt, Car& car) = 0;
};

#endif //CARSIM_ICONTROLLER_HPP