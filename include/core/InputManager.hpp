#ifndef CARSIM_INPUTMANAGER_HPP
#define CARSIM_INPUTMANAGER_HPP

#include "core/EventBus.hpp"
#include "types/events.hpp"
#include "threepp/input/KeyListener.hpp"

class InputManager : public threepp::KeyListener {
public:
    explicit InputManager(EventBus& bus)
        : bus_(bus) {
    }
    void onKeyPressed(const threepp::KeyEvent evt) override {
        bus_.publish(KeyPressedEvent{evt});
        if (evt.key == threepp::Key::P) {
            bus_.publish(PauseEvent{});
        } if (evt.key == threepp::Key::ENTER) {
            bus_.publish(ResumeEvent{});
        }
    }
    void onKeyReleased(const threepp::KeyEvent evt) override {
        bus_.publish(KeyReleasedEvent{evt});
    }

private:
    EventBus& bus_;
};

#endif //CARSIM_INPUTMANAGER_HPP