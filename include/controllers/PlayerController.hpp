#ifndef CARSIM_PLAYERCONTROLLER_HPP
#define CARSIM_PLAYERCONTROLLER_HPP

#include "threepp/input/KeyListener.hpp"
#include "IController.hpp"
#include "core/EventBus.hpp"
#include "logic/Car.hpp"

class PlayerController : public IController {

public:
    EventBus& bus_;
    Car& car_;
    explicit PlayerController(EventBus& bus ,Car& car);

    void onKeyPressed(threepp::KeyEvent evt) const;

    void onKeyReleased(threepp::KeyEvent evt) const;

    void update(float dt, Car& car) override {}
};

#endif //CARSIM_PLAYERCONTROLLER_HPP