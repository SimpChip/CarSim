//
// Created by Mella on 18.10.2025.
//

#ifndef CARSIM_CARLOGIC_HPP
#define CARSIM_CARLOGIC_HPP

#pragma once
#include <threepp/threepp.hpp>

using Vector3 = threepp::Vector3;
#include "CarLogic.hpp"
#include "iostream"
#include <cmath>

class CarLogic {

    public:
        Vector3 position_ = Vector3(0, 0, 0);
        Vector3 previousPosition_ = Vector3(0, 0, 0);
        Vector3 velocity_ = Vector3(0, 0, 0);
        float rotation_ = 0;

        float accelerationInput = 0;
        float steeringInput = 0;

        float worldScale = 200.0f;
        float maxSpeed = 50.0f * worldScale;
        float accelerationSpeed = 5.0f * worldScale;
        float maxTurnRate = 2;

        float sideFriction = 0.94f;
        float drag = 0.98;

        CarLogic(Vector3 startPos, float rotation) : position_(startPos) , rotation_(rotation)
        {}

        void setPosition(Vector3 position) { position_ = position; }
        Vector3 getPosition() const { return position_; }
        void setPrevPosition(Vector3 previousPosition) { previousPosition_ = previousPosition; }
        Vector3 getPreviousPosition() const { return previousPosition_; }

        float getRotation() { return rotation_; }
        Vector3 getVelocity() const { return velocity_; }

        void setAccelerationInput(float amount) { accelerationInput = std::clamp(amount, -1.0f, 1.0f); }
        void setSteeringInput(float amount) { steeringInput = std::clamp(amount, -1.0f, 1.0f); }

        void applyBoost(float boostAmount) { velocity_ += forwardVector() * boostAmount; }
        void applyBraking(float factor, float dt) { velocity_ *= std::pow(factor, dt * 60.0f);}
        void stopCar() { velocity_ = Vector3(0,0,0); }

        void updateRotation(float dt)
        {
            float speed = velocity_.length();
            float speedFactor = std::min(speed / maxSpeed, 1.0f);
            rotation_ += steeringInput * maxTurnRate * dt * (1.0f - 0.5f * speedFactor);
        }

        void updateVelocity(float dt)
        {
            velocity_ += forwardVector() * (accelerationInput * accelerationSpeed * dt);
            std::cout << velocity_.length() << std::endl;

            if (velocity_.length() < 0.1f)
            {
                velocity_ = Vector3(0,0,0);
            }
            //
            velocity_ *= std::pow(drag, dt * 60.0f);
        }

        void applySideFriction(float dt)
            {
                Vector3 forward = forwardVector();
                float forwardSpeed = velocity_.dot(forward);
                Vector3 lateral = velocity_ - forward * forwardSpeed;
                lateral *= sideFriction;
                velocity_ = forward * forwardSpeed + lateral;
            }

        void clampSpeed()
            {
                float speed = velocity_.length();
                if (speed > maxSpeed)
                    velocity_ = velocity_.normalize() * maxSpeed;
            }
        void update(float dt)
        {
            if (dt <= 0.0f) return;
            previousPosition_ = position_;
            updateRotation(dt);
            updateVelocity(dt);
            applySideFriction(dt);
            clampSpeed();
            position_ += velocity_ * dt;
        }


        Vector3 forwardVector() const
        {
            return Vector3(std::sin(rotation_), 0, std::cos(rotation_));
        }





};

#endif //CARSIM_CARLOGIC_HPP