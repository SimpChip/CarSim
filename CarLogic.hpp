//
// Created by Mella on 18.10.2025.
//

#ifndef CARSIM_CARLOGIC_HPP
#define CARSIM_CARLOGIC_HPP

#pragma once
#include <threepp/threepp.hpp>
using Vector3 = threepp::Vector3;

class CarLogic
{
public:
    explicit CarLogic(threepp::Vector3 startPos = Vector3(0,0,0), float rotation = 0.0f);

    [[nodiscard]] threepp::Vector3& getPosition() const;
    [[nodiscard]] Vector3 getPreviousPosition() const;
    [[nodiscard]] Vector3 getVelocity() const;
    [[nodiscard]] float& getRotation() const;

    void setPosition(const Vector3& pos);
    void setVelocity(const Vector3& vel);

    void setMaxSpeed(float speed);
    void setAccelerationSpeed(float accel);
    void setMaxTurnRate(float rate);
    void setSideFriction(float friction);

    void setAccelerationInput(float amount);
    void setSteeringInput(float amount);
    void applyBoost(float boostAmount);

    void stopCar();
    void update(float dt);
    void applyBraking(float factor);

private:
    threepp::Vector3& position;
    Vector3 previousPosition;
    Vector3 velocity;
    float& rotation;

    float accelerationInput;
    float steeringInput;

    float maxSpeed;
    float accelerationSpeed;
    float maxTurnRate;
    float sideFriction;
    float drag;

    [[nodiscard]] Vector3 forwardVector() const;
    void updateRotation(float dt);
    void updateVelocity(float dt);
    void applySideFriction();
    void clampSpeed();
};

#endif //CARSIM_CARLOGIC_HPP