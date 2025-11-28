#include "controllers/PlayerController.hpp"

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
