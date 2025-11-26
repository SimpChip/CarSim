//
// Created by Mella on 18.10.2025.
//

#ifndef CARSIM_CARLOGIC_HPP
#define CARSIM_CARLOGIC_HPP

#include <threepp/threepp.hpp>
#include "iostream"
#include <cmath>
#include "types/data.hpp"



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

    static void projectOBB(const OBB& obb, const Vector3& axis, float& min, float& max) {
        const auto corners = obb.getCorners();
        min = max = corners[0].dot(axis);
        for (size_t i = 1; i < 4; ++i) {
            const float projection = corners[i].dot(axis);
            if (projection < min) min = projection;
            if (projection > max) max = projection;
        }
    }

    [[nodiscard]] bool intersects(const OBB& other) const {
        Vector3 axes[4] = {
            this->forward,
            this->right,
            other.forward,
            other.right
        };
        for (auto& axis : axes) {
            const Vector3 normalizedAxis = axis.normalize();

            float minA, maxA;
            projectOBB(*this, normalizedAxis, minA, maxA);

            float minB, maxB;
            projectOBB(other, normalizedAxis, minB, maxB);
            if (maxA < minB || maxB < minA) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::array<Vector3, 4> getCorners() const {
        return {
            center + forward * halfExtents.y + right * halfExtents.x,
            center + forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y + right * halfExtents.x
            };
    }
};

class CarLogic {

    public:
        //redcar 2.1 4.66
        //colltruck 2 4

        Vector3 velocity_ = Vector3(0, 0, 0);

        CarLogic(const Vector3 startPos, const float rotation, const float scale) :position_(startPos),  previousPosition_(startPos), rotation_(rotation), scale_(scale) {
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

        void setPosition(const Vector3 position) { position_ = position; }
        [[nodiscard]] Vector3 getPosition() const { return position_; }
        void setPrevPosition(const Vector3 previousPosition) { previousPosition_ = previousPosition; }
        [[nodiscard]] Vector3 getPreviousPosition() const { return previousPosition_; }

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

        void setBoostFactor(const float i) { boostFactor_ = i; }

        void applyVelocity(const threepp::Vector3 vel) { velocity_ += vel;}
        void applyBounce() {velocity_ = velocity_.negate() * 0.8;}

        void stopCar() { velocity_ = Vector3(0,0,0); }

        [[nodiscard]] OBB getOrientedBoundingBox() const {
            const auto fwd = forwardVector().normalize();
            const auto right = Vector3(-fwd.y, fwd.x, 0);
            const threepp::Vector3 halfExtents(width_*scale_ / 2.0f, height_*scale_ / 2.0f, 0);
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

        [[nodiscard]] CarData snapshot() const {
            return { position_, previousPosition_, forwardVector(), rotation_, velocity_.length(), currentSteeringInput, boostFactor_,  };
        }

        [[nodiscard]] threepp::Vector3 forwardVector() const
        {
            return {std::sin(rotation_), std::cos(rotation_),0 };
        }
    private:
        float width_ = 2.f;
        float height_ = 4.4f;
        Vector3 position_;
        Vector3 previousPosition_;

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
                const Vector3 forward = forwardVector();
                const float dotProduct = velocity_.dot(forward);
                const float directionMultiplier = (dotProduct < 0.0f) ? -1.0f : 1.0f;
                rotation_ += currentSteeringInput * dt * speedFactor * directionMultiplier;
            }
        }

        void updateVelocity(const float dt) {
            if (boostFactor_ > 1) boostFactor_ *= 0.994f;

            const float oldSpeed = velocity_.length();
            float finalAccelerationMagnitude;

            if (accelerationInput >= 0.0f) {
                const float throttleReduction = 1.0f - (oldSpeed / (baseMaxSpeed * maxSpeedMultiplier_));
                finalAccelerationMagnitude = accelerationInput * accelerationSpeed * throttleReduction * boostFactor_;
            } else {
                finalAccelerationMagnitude = accelerationInput * brakeSpeed;
            }

            const Vector3 acceleration = forwardVector() * finalAccelerationMagnitude;
            velocity_ += acceleration * dt;

            const float speed = velocity_.length();
            if (speed < 0.1f) {
                velocity_ = Vector3(0, 0, 0);
                return;
            }

            Vector3 dragDirection = velocity_;
            dragDirection.normalize();

            const float quadraticDrag = speed * speed * drag * dragMultiplier_;
            float linearDrag = 0.0f;
            if (accelerationInput <= 0.0f) {
                linearDrag = speed * rollingResistance * rollingResistanceMultiplier_;
            }
            const float totalDragMagnitude = quadraticDrag + linearDrag;
            const Vector3 dragForce = dragDirection * totalDragMagnitude;
            velocity_ -= dragForce * dt;
        }

        void applySideFriction(float dt) {
            const Vector3 forward = forwardVector();
            const float forwardSpeed = velocity_.dot(forward);
            Vector3 lateral = velocity_ - forward * forwardSpeed;


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
                Vector3 n = velocity_;
                n.normalize();
                velocity_ = n * effectiveMaxSpeed;
            }
        }
};

#endif //CARSIM_CARLOGIC_HPP