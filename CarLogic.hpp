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
        //Position
        Vector3 position_ = Vector3(0, 0, 0);
        Vector3 previousPosition_ = Vector3(0, 0, 0);
        Vector3 velocity_ = Vector3(0, 0, 0);
        float rotation_ = 0;
        float accelerationInput = 0;

        //steering
        float targetSteeringInput = 0.0f;
        float currentSteeringInput = 0.0f;
        bool isSteeringLeftActive = false;
        bool isSteeringRightActive = false;

        // Constants
        float worldScale = 100.0f;
        float accelerationSpeed = 5.0f * worldScale;
        float maxSpeed = 10.0f * worldScale;
        float steeringEaseRate = 4.f;
        float boostFactor_ = 1;
        float sideFriction = 0.94f;
        float drag = 0.995;

        CarLogic(Vector3 startPos, float rotation) : position_(startPos) , rotation_(rotation)
        {}

        void setPosition(Vector3 position) { position_ = position; }
        Vector3 getPosition() const { return position_; }
        void setPrevPosition(Vector3 previousPosition) { previousPosition_ = previousPosition; }
        Vector3 getPreviousPosition() const { return previousPosition_; }

        float getRotation() { return rotation_; }
        float getCurrentSteeringInput() { return currentSteeringInput; }
        Vector3 getVelocity() const { return velocity_; }

        void setAccelerationInput(float amount) { accelerationInput = std::clamp(amount, -1.0f, 1.0f); }

        void applyBoost(float boostAmount) { velocity_ += forwardVector() * boostAmount; }
        void applyBraking(float factor, float dt) { velocity_ *= std::pow(factor, dt * 60.0f);}
        void applyBounce() {velocity_ = velocity_.negate() * 0.8;}
        void stopCar() { velocity_ = Vector3(0,0,0); }

        void updateTargetSteering()
        {
            float target = 0.0f;
            if (isSteeringLeftActive && !isSteeringRightActive) {
                target = -1.0f; // Left
            } else if (isSteeringRightActive && !isSteeringLeftActive) {
                target = 1.0f; // Right
            }
            targetSteeringInput = std::clamp(target, -1.0f, 1.0f);
        }

        void setSteeringLeftActive(bool state) {
                isSteeringLeftActive = state;
                updateTargetSteering();
            }

        void setSteeringRightActive(bool state) {
                isSteeringRightActive = state;
                updateTargetSteering();
            }

        void updateRotation(float dt)
        {
            float speed = velocity_.length();
            float step = steeringEaseRate * dt;
            if (currentSteeringInput < targetSteeringInput) {
                currentSteeringInput = std::min(currentSteeringInput + step, targetSteeringInput);
            } else if (currentSteeringInput > targetSteeringInput) {
                currentSteeringInput = std::max(currentSteeringInput - step, targetSteeringInput);
            }
            if (speed > 0.1f) {
                float speedFactor = std::pow(speed, 0.7)/40;
                Vector3 forward = forwardVector();
                //std::cout << forward << std::endl;
                float dotProduct = velocity_.dot(forward);
                float directionMultiplier = (dotProduct < 0.0f) ? -1.0f : 1.0f;
                rotation_ += currentSteeringInput * dt * speedFactor * directionMultiplier;
            }
        }

        void updateVelocity(float dt)
        {
            if (boostFactor_ > 1) { boostFactor_ *= 0.995; }
            float speed = velocity_.length();

            Vector3 acceleration = forwardVector() * (accelerationInput * accelerationSpeed * boostFactor_);
            velocity_ += acceleration * dt;


            if (velocity_.length() < 0.1f) {
                velocity_ = Vector3(0,0,0);
            }

            Vector3 dragForce = velocity_ * drag;
            velocity_ -= dragForce * dt;
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
            return Vector3(std::sin(rotation_), std::cos(rotation_),0 );
        }

        void setBoostFactor(float i) {
            boostFactor_ = i;
        }
};

#endif //CARSIM_CARLOGIC_HPP