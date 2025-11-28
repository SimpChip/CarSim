#include "../../include/logic/CarLogic.hpp"
#include <algorithm>
#include <iostream>

CarLogic::CarLogic(const threepp::Vector3& startPos, const float rotation, const float scale)
      : ICollidable(startPos, CollisionType::Car),rotation_(rotation), scale_(scale) {
    const float scaleFactor = scale * scale;
    accelerationSpeed = 4.0f * scaleFactor;
    brakeSpeed = 8.0f * scaleFactor;
    baseMaxSpeed = 10.0f * scaleFactor;
    rollingResistance = 0.5f;
    drag = 0.0002f;
}
threepp::Vector3 CarLogic::getVelocity() const { return velocity_; }

threepp::Vector3 CarLogic::forwardVector() const {
    return {std::sin(rotation_), std::cos(rotation_), 0};
}

OBB2D CarLogic::getBoundingBox() const {
    const auto fwd = forwardVector();
    return OBB2D{
        threepp::Vector2{position_.x, position_.y},
        threepp::Vector2{size_.x/2, size_.y/2},
        threepp::Vector2{fwd.x, fwd.y}
    };
}

void CarLogic::update(const float dt) {
    if (dt <= 0.0f) return;
    prevPosition_ = position_;
    updateRotation(dt);
    updateVelocity(dt);
    applySideFriction(dt);
    clampSpeed();
    position_ += velocity_ * dt;
}

void CarLogic::onCollision(const std::shared_ptr<ICollidable>& other) {
    switch (other->getCollisionType()) {
        case CollisionType::Car:
            if (const auto otherCar = std::dynamic_pointer_cast<CarLogic>(other)) {
                resolveCollisionImpulse(otherCar);
            }
            break;
        case CollisionType::Obstacle:
            break;
        case CollisionType::SpeedBoost:
            boostFactor_ += 2;
            break;
        case CollisionType::SlowBoost:
            boostFactor_ = boostFactor_ / 2;
            break;
        default: break;
    }
}

void CarLogic::resolveCollisionImpulse(const std::shared_ptr<CarLogic>& other) {
    threepp::Vector3 normal = position_ - other->getPosition();
    if (normal.length() < 1e-6f) {
        return;
    }
    normal.normalize();
    const threepp::Vector3 relativeVelocity = velocity_ - other->getVelocity();
    const float velocityAlongNormal = relativeVelocity.dot(normal);
    if (velocityAlongNormal > 0) return;

    constexpr float e = 0.1f;
    const float J = -(1.0f + e) * velocityAlongNormal;
    const threepp::Vector3 impulse = normal * J;
    applyVelocity(impulse);
}

void CarLogic::setSteeringLeftActive(const bool state) {
    isSteeringLeftActive = state;
    updateTargetSteering();
}

void CarLogic::setSteeringRightActive(const bool state) {
    isSteeringRightActive = state;
    updateTargetSteering();
}

void CarLogic::setAccelerationInput(const float amount) {
    accelerationInput = std::clamp(amount, -1.0f, 1.0f);
}

void CarLogic::applyVelocity(const threepp::Vector3& vel) {
    velocity_ += vel;
}


void CarLogic::setDragMultiplier(const float m) { dragMultiplier_ = m; }
void CarLogic::setGripMultiplier(const float m) { gripMultiplier_ = m; }
void CarLogic::setRollingResistanceMultiplier(const float m) { rollingResistanceMultiplier_ = m; }
void CarLogic::setMaxSpeedMultiplier(const float m) { maxSpeedMultiplier_ = m; }

void CarLogic::updateTargetSteering() {
    float target = 0.0f;
    if (isSteeringLeftActive && !isSteeringRightActive) target = -1.0f;
    else if (isSteeringRightActive && !isSteeringLeftActive) target = 1.0f;
    targetSteeringInput = std::clamp(target, -1.0f, 1.0f);
}

void CarLogic::updateRotation(const float dt) {
    const float speed = velocity_.length();
    const float step = steeringEaseRate * dt;
    if (currentSteeringInput < targetSteeringInput)
        currentSteeringInput = std::min(currentSteeringInput + step, targetSteeringInput);
    else if (currentSteeringInput > targetSteeringInput)
        currentSteeringInput = std::max(currentSteeringInput - step, targetSteeringInput);

    if (speed > 0.1f * scale_ * scale_) {
        constexpr float maxRotationRate = 2.0f;
        const float speedFactor = maxRotationRate / (1.0f + speed / (baseMaxSpeed * maxSpeedMultiplier_));
        const threepp::Vector3 forward = forwardVector();
        const float dotProduct = velocity_.dot(forward);
        const float directionMultiplier = (dotProduct < 0.0f) ? -1.0f : 1.0f;
        rotation_ += currentSteeringInput * dt * speedFactor * directionMultiplier;
    }
}

void CarLogic::updateVelocity(const float dt) {
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

void CarLogic::applySideFriction(const float dt) {
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

void CarLogic::clampSpeed() {
    const float effectiveMaxSpeed = baseMaxSpeed * maxSpeedMultiplier_;
    const float speed = velocity_.length();
    if (speed > effectiveMaxSpeed) {
        threepp::Vector3 n = velocity_;
        n.normalize();
        velocity_ = n * effectiveMaxSpeed;
    }
}


CarData CarLogic::snapshot() const {
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