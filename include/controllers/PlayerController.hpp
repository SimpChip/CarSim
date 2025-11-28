#ifndef CARSIM_PLAYERCONTROLLER_HPP
#define CARSIM_PLAYERCONTROLLER_HPP

#include "threepp/input/KeyListener.hpp"
#include "IController.hpp"
#include "logic/CarLogic.hpp"

class PlayerController : public IController {

public:
    CarLogic& car_;
    explicit PlayerController(CarLogic& car) : car_(car) {}

    void onKeyPressed(threepp::KeyEvent evt) const;

    void onKeyReleased(threepp::KeyEvent evt) const;

    void update(float dt, CarLogic& car) override {}
};

#endif //CARSIM_PLAYERCONTROLLER_HPP