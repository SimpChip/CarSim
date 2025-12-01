#include "controllers/PlayerController.hpp"

#include <iostream>

#include "types/events.hpp"

PlayerController::PlayerController(EventBus& bus, Car& car) : bus_(bus), car_(car) {
    bus_.subscribe<KeyPressedEvent>([this](const KeyPressedEvent& e){
        onKeyPressed(e.evt);
    });
    bus_.subscribe<KeyReleasedEvent>([this](const KeyReleasedEvent& e){
        onKeyReleased(e.evt);
    });
}



void PlayerController::onKeyPressed(const threepp::KeyEvent evt) const {
    switch(evt.key) {
        case threepp::Key::W: car_.setAccelerationInput(1.0f); break;
        case threepp::Key::S: car_.setAccelerationInput(-1.0f); break;
        case threepp::Key::A: car_.setSteeringLeftActive(true); break;
        case threepp::Key::D: car_.setSteeringRightActive(true); break;
        default: ;
    }
}

void PlayerController::onKeyReleased(const threepp::KeyEvent evt) const {
    switch(evt.key) {
        case threepp::Key::W:
        case threepp::Key::S: car_.setAccelerationInput(0.0f); break;
        case threepp::Key::A: car_.setSteeringLeftActive(false); break;
        case threepp::Key::D: car_.setSteeringRightActive(false); break;
        default: ;
    }
}
