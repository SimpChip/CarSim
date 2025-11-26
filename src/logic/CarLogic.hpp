//
// Created by Mella on 18.10.2025.
//

#ifndef CARSIM_CARLOGIC_HPP
#define CARSIM_CARLOGIC_HPP

#include <threepp/threepp.hpp>
#include "iostream"
#include <cmath>

#include "CollisonManager.hpp"
#include "types/data.hpp"
#include "data/OBB.hpp"


class CarLogic final : public ICollidable {
    public:
        //redcar 2.1 4.66
        //colltruck 2 4
        threepp::Vector3 velocity_ = threepp::Vector3(0, 0, 0);

        CarLogic(const threepp::Vector3 startPos, const float rotation, const float scale) :position_(startPos),  previousPosition_(startPos), rotation_(rotation), scale_(scale) {
            const float scaleFactor = scale * scale;
            accelerationSpeed = 2.0f * scaleFactor;
            brakeSpeed = 4.0f * scaleFactor;
            baseMaxSpeed = 7.0f * scaleFactor;
            rollingResistance = 0.5f;
            drag = 0.0002f;
        }

        void setDragMultiplier(float m) { dragMultiplier_ = m; }
        void setGripMultiplier(float m) { gripMultiplier_ = m; }
        void setRollingResistanceMultiplier(float m) { rollingResistanceMultiplier_ = m; }
        void setMaxSpeedMultiplier(float m) { maxSpeedMultiplier_ = m; }

        void setPosition(const threepp::Vector3 position) { position_ = position; }
        void setPrevPosition(const threepp::Vector3 previousPosition) { previousPosition_ = previousPosition; }
        [[nodiscard]] threepp::Vector3 getPosition() const override { return position_; }
        [[nodiscard]] threepp::Vector3 getPreviousPosition() const { return previousPosition_; }

        void setSteeringLeftActive(const bool state) {
                isSteeringLeftActive = state;
                updateTargetSteering();
        }
        void setSteeringRightActive(const bool state) {
                isSteeringRightActive = state;
                updateTargetSteering();
        }

        void setAccelerationInput(const float amount) {
            accelerationInput = std::clamp(amount, -1.0f, 1.0f);
        }


        [[nodiscard]] threepp::Vector3 getVelocity() const { return velocity_; }

        void applyVelocity(const threepp::Vector3 vel) { velocity_ += vel;}

        [[nodiscard]] threepp::Vector3 forwardVector() const
        {
            return {std::sin(rotation_), std::cos(rotation_),0 };
        }

        [[nodiscard]] CollisionType getCollisonType() const override { return CollisionType::Car; }

        [[nodiscard]] OBB getBoundingBox() const override {
            const auto fwd = forwardVector().normalize();
            const auto right = threepp::Vector3(-fwd.y, fwd.x, 0);
            const threepp::Vector3 halfExtents(size_.x*scale_ / 2.0f, size_.y*scale_ / 2.0f, 0);
            return {position_, halfExtents, fwd, right};
        }

        void update(const float dt) {
            if (dt <= 0.0f) return;
            previousPosition_ = position_;
            updateRotation(dt);
            updateVelocity(dt);
            applySideFriction(dt);
            clampSpeed();
            position_ += velocity_ * dt;
        }

        void onCollision(ICollidable* other) override {
            switch (other->getCollisonType()) {
                case  CollisionType::Car:
                    resolveCollisionImpulse(*dynamic_cast<CarLogic*>(other));
                    break;
                case  CollisionType::Obstacle:
                    break;
                case CollisionType::SpeedBoost:
                    boostFactor_ += 2;
                    break;
                case CollisionType::SlowBoost:
                    boostFactor_ = boostFactor_/2;
                default: ;
            }
        }

        void resolveCollisionImpulse(CarLogic& other) {
            threepp::Vector3 normal = position_ - other.getPosition();
            normal.normalize();
            const threepp::Vector3 relativeVelocity = velocity_ - other.getVelocity();

            const float velocityAlongNormal = relativeVelocity.dot(normal);

            if (velocityAlongNormal > 0) return;

            constexpr float e = 0.1f;
            const float J = -(1.0f + e) * velocityAlongNormal;
            const threepp::Vector3 impulse = normal * J;

            applyVelocity(impulse);
            other.applyVelocity((threepp::Vector3(0,0,0) - impulse)) ;
        }

        [[nodiscard]] CarData snapshot() const {
            CarData data;
            data.position = position_;
            data.forward = forwardVector();
            data.rotation = rotation_;
            data.boostFactor = boostFactor_;
            data.speed = velocity_.length();
            data.carSize = size_;
            data.steering = currentSteeringInput;
            return data;
        }


    private:
        threepp::Vector3 size_ = {2, 4, 1.5};
        threepp::Vector3 position_;
        threepp::Vector3 previousPosition_;

        float rotation_ = 0;

        float accelerationInput = 0;
        float targetSteeringInput = 0.0f;
        float currentSteeringInput = 0.0f;
        bool isSteeringLeftActive = false;
        bool isSteeringRightActive = false;

        float scale_;
        float accelerationSpeed;
        float brakeSpeed;
        float baseMaxSpeed;
        float steeringEaseRate = 6.f;
        float boostFactor_ = 1;

        float rollingResistance;
        float drag;

        float dragMultiplier_ = 1.0f;
        float gripMultiplier_ = 1.0f;
        float rollingResistanceMultiplier_ = 1.0f;
        float maxSpeedMultiplier_ = 1.0f;

        void updateTargetSteering() {
            float target = 0.0f;
            if (isSteeringLeftActive && !isSteeringRightActive) {
                target = -1.0f; // Left
            } else if (isSteeringRightActive && !isSteeringLeftActive) {
                target = 1.0f; // Right
            }
            targetSteeringInput = std::clamp(target, -1.0f, 1.0f);
        }

        void updateRotation(const float dt) {
            const float speed = velocity_.length();
            const float step = steeringEaseRate * dt;
            if (currentSteeringInput < targetSteeringInput) {
                currentSteeringInput = std::min(currentSteeringInput + step, targetSteeringInput);
            } else if (currentSteeringInput > targetSteeringInput) {
                currentSteeringInput = std::max(currentSteeringInput - step, targetSteeringInput);
            }
            if (speed > 0.1f * scale_ * scale_) {
                constexpr float maxRotationRate = 2.0f;
                const float speedFactor = maxRotationRate / (1.0f + speed / (baseMaxSpeed * maxSpeedMultiplier_));
                const threepp::Vector3 forward = forwardVector();
                const float dotProduct = velocity_.dot(forward);
                const float directionMultiplier = (dotProduct < 0.0f) ? -1.0f : 1.0f;
                rotation_ += currentSteeringInput * dt * speedFactor * directionMultiplier;
            }
        }

        void updateVelocity(const float dt) {
            if (boostFactor_ > 1) boostFactor_ *= 0.994f;
            else if (boostFactor_ < 1) boostFactor_ *= 1.1f;

            const float oldSpeed = velocity_.length();
            float finalAccelerationMagnitude;

            if (accelerationInput >= 0.0f) {
                const float throttleReduction = 1.0f - (oldSpeed / (baseMaxSpeed * maxSpeedMultiplier_));
                finalAccelerationMagnitude = accelerationInput * accelerationSpeed * throttleReduction * boostFactor_;
            } else {
                finalAccelerationMagnitude = accelerationInput * brakeSpeed;
            }

            const threepp::Vector3 acceleration = forwardVector() * finalAccelerationMagnitude;
            velocity_ += acceleration * dt;

            const float speed = velocity_.length();
            if (speed < 0.1f) {
                velocity_ = threepp::Vector3(0, 0, 0);
                return;
            }

            threepp::Vector3 dragDirection = velocity_;
            dragDirection.normalize();

            const float quadraticDrag = speed * speed * drag * dragMultiplier_;
            float linearDrag = 0.0f;
            if (accelerationInput <= 0.0f) {
                linearDrag = speed * rollingResistance * rollingResistanceMultiplier_;
            }
            const float totalDragMagnitude = quadraticDrag + linearDrag;
            const threepp::Vector3 dragForce = dragDirection * totalDragMagnitude;
            velocity_ -= dragForce * dt;
        }

        void applySideFriction(float dt) {
            const threepp::Vector3 forward = forwardVector();
            const float forwardSpeed = velocity_.dot(forward);
            threepp::Vector3 lateral = velocity_ - forward * forwardSpeed;


            constexpr float BASE_GRIP = 0.98f;
            constexpr float DRIFT_SLIDE = 0.85f;
            const float steeringMagnitude = std::abs(currentSteeringInput);

            float dynamicFrictionFactor = BASE_GRIP - (BASE_GRIP - DRIFT_SLIDE) * steeringMagnitude;

            dynamicFrictionFactor *= gripMultiplier_;
            dynamicFrictionFactor = std::clamp(dynamicFrictionFactor, 0.0f, 1.0f);

            lateral *= dynamicFrictionFactor;
            velocity_ = forward * forwardSpeed + lateral;
        }

        void clampSpeed() {
            const float effectiveMaxSpeed = baseMaxSpeed * maxSpeedMultiplier_;
            const float speed = velocity_.length();
            if (speed > effectiveMaxSpeed) {
                threepp::Vector3 n = velocity_;
                n.normalize();
                velocity_ = n * effectiveMaxSpeed;
            }
        }
};

#endif //CARSIM_CARLOGIC_HPP