#ifndef CARSIM_ICONTROLLER_HPP
#define CARSIM_ICONTROLLER_HPP

#include "entities/Car.hpp"

class IController {
public:
    virtual ~IController() = default;
    virtual void update(float dt, Car& car) = 0;
};

#endif //CARSIM_ICONTROLLER_HPP