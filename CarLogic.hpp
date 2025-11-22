//
// Created by Mella on 18.10.2025.
//

#ifndef CARSIM_CARLOGIC_HPP
#define CARSIM_CARLOGIC_HPP

#include <threepp/threepp.hpp>
#include "iostream"
#include <cmath>

using Vector3 = threepp::Vector3;

struct OBB {
    Vector3 center;
    Vector3 halfExtents;
    Vector3 forward;
    Vector3 right;

    [[nodiscard]] bool containsPoint(const Vector3& point) const {
        const Vector3 local = point - center;
        const float xProj = local.dot(right);
        const float yProj = local.dot(forward);
        return std::abs(xProj) <= halfExtents.x && std::abs(yProj) <= halfExtents.y;
    }


    [[nodiscard]] std::array<Vector3, 4> getCorners() const {
        return {
            center + forward * halfExtents.y + right * halfExtents.x, // front-right
            center + forward * halfExtents.y - right * halfExtents.x, // front-left
            center - forward * halfExtents.y - right * halfExtents.x, // back-left
            center - forward * halfExtents.y + right * halfExtents.x  // back-right
            };
    }
};

class CarLogic {

    public:
        //Position
        float width_ = 2;
        float height_ = 4;
        Vector3 position_;
        Vector3 previousPosition_;
        Vector3 velocity_ = Vector3(0, 0, 0);
        float accelerationInput = 0;
        float rotation_ = 0;


        //steering
        float targetSteeringInput = 0.0f;
        float currentSteeringInput = 0.0f;
        bool isSteeringLeftActive = false;
        bool isSteeringRightActive = false;

        // Constants
        float scale_ = 1.0f;

        float accelerationSpeed = 8.0f * scale_ * scale_;
        float maxSpeed = 50.0f * scale_ * scale_;
        float steeringEaseRate = 4.f;
        float boostFactor_ = 1;
        float sideFriction = 0.94f;
        float drag = 0.995;

        CarLogic(const Vector3 startPos, const float rotation, const float scale) : previousPosition_(startPos), position_(startPos) , rotation_(rotation), scale_(scale)
        {}

        void setPosition(const Vector3 position) { position_ = position; }
        [[nodiscard]] Vector3 getPosition() const { return position_; }
        void setPrevPosition(const Vector3 previousPosition) { previousPosition_ = previousPosition; }
        [[nodiscard]] Vector3 getPreviousPosition() const { return previousPosition_; }

        [[nodiscard]] OBB getOrientedBoundingBox() const {
            const auto fwd = forwardVector().normalize();
            const auto right = Vector3(-fwd.y, fwd.x, 0);
            const threepp::Vector3 halfExtents(width_*scale_ / 2.0f, height_*scale_ / 2.0f, 0);
            return {position_, halfExtents, fwd, right};
        }

        [[nodiscard]] float getRotation() const { return rotation_; }
        [[nodiscard]] float getCurrentSteeringInput() const { return currentSteeringInput; }
        [[nodiscard]] Vector3 getVelocity() const { return velocity_; }

        void setAccelerationInput(const float amount) {
            accelerationInput = std::clamp(amount, -1.0f, 1.0f);
        }

        void applyBraking(const float factor, const float dt) { velocity_ *= std::pow(factor, dt * 60.0f);}
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

        void setSteeringLeftActive(const bool state) {
                isSteeringLeftActive = state;
                updateTargetSteering();
            }

        void setSteeringRightActive(const bool state) {
                isSteeringRightActive = state;
                updateTargetSteering();
            }

        void updateRotation(const float dt)
        {
            float speed = velocity_.length();
            float step = steeringEaseRate * dt;
            if (currentSteeringInput < targetSteeringInput) {
                currentSteeringInput = std::min(currentSteeringInput + step, targetSteeringInput);
            } else if (currentSteeringInput > targetSteeringInput) {
                currentSteeringInput = std::max(currentSteeringInput - step, targetSteeringInput);
            }
            if (speed > 0.1f) {
                const float speedFactor = std::pow(speed, 0.4)/10;
                const Vector3 forward = forwardVector();
                //std::cout << forward << std::endl;
                const float dotProduct = velocity_.dot(forward);
                const float directionMultiplier = (dotProduct < 0.0f) ? -1.0f : 1.0f;
                rotation_ += currentSteeringInput * dt * speedFactor * directionMultiplier;
            }
        }

        void updateVelocity(const float dt)
        {
            if (boostFactor_ > 1) { boostFactor_ *= 0.994; }

            const Vector3 acceleration = forwardVector() * (accelerationInput * accelerationSpeed * boostFactor_);
            velocity_ += acceleration * dt;


            if (velocity_.length() < 0.1f) {
                velocity_ = Vector3(0,0,0);
            }

            const Vector3 dragForce = velocity_ * drag;
            velocity_ -= dragForce * dt;
        }

        void applySideFriction(float dt)
            {
                const Vector3 forward = forwardVector();
                const float forwardSpeed = velocity_.dot(forward);
                Vector3 lateral = velocity_ - forward * forwardSpeed;
                lateral *= sideFriction;
                velocity_ = forward * forwardSpeed + lateral;
            }

        void clampSpeed()
            {
                if (const float speed = velocity_.length(); speed > maxSpeed)
                    velocity_ = velocity_.normalize() * maxSpeed;
            }
        void update(const float dt)
        {
            if (dt <= 0.0f) return;
            previousPosition_ = position_;
            updateRotation(dt);
            updateVelocity(dt);
            applySideFriction(dt);
            clampSpeed();
            position_ += velocity_ * dt;
            //std::cout << position_ << std::endl;
        }


        [[nodiscard]] Vector3 forwardVector() const
        {
            return {std::sin(rotation_), std::cos(rotation_),0 };
        }

        void setBoostFactor(const float i) {
            boostFactor_ = i;
        }
};

#endif //CARSIM_CARLOGIC_HPP