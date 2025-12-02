#include "entities/Car.hpp"
#include <algorithm>
#include <iostream>
#include "types/gameSnapshots.hpp"

Car::Car(const threepp::Vector3& startPos, const float rotation, const float scale)
      : ICollidable(CollisionType::Car, startPos, rotation), scale_(scale) {
    const float scaleFactor = scale * scale;
    accelerationSpeed = 4.0f * scaleFactor;
    brakeSpeed = 8.0f * scaleFactor;
    baseMaxSpeed = 10.0f * scaleFactor;
    rollingResistance = 0.5f;
    drag = 0.0002f;
}
threepp::Vector3 Car::getVelocity() const { return velocity_; }

threepp::Vector3 Car::forwardVector() const {
    return {std::sin(rotation_), std::cos(rotation_), 0};
}

OBB2D Car::getBoundingBox() const {
    const auto fwd = forwardVector();
    return OBB2D::create(
        threepp::Vector2{position_.x, position_.y},
        threepp::Vector2{size_.x/2, size_.y/2},
        threepp::Vector2{fwd.x, fwd.y}
    );
}

void Car::update(const float dt) {
    if (dt <= 0.0f) return;
    prevPosition_ = position_;
    updateRotation(dt);
    updateVelocity(dt);
    applySideFriction(dt);
    clampSpeed();
    position_ += velocity_ * dt;
}

void Car::onCollision(const std::shared_ptr<ICollidable>& other, float dt) {
    switch (other->getCollisionType()) {
        case CollisionType::Car:
            if (const auto otherCar = std::dynamic_pointer_cast<Car>(other)) {
                resolveCollisionImpulse(otherCar, dt);
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

void Car::resolveCollisionImpulse(const std::shared_ptr<Car>& other, float dt) {
    threepp::Vector3 collisionDir = position_ - other->position_;
    const float distance = collisionDir.length();
    if (distance < 0.001f) {
        std::cout << "no distance" << std::endl;
        position_ += velocity_ * -1 * dt;
        return;
    };
    collisionDir.normalize();

    const float combinedHalfSize = (size_.length() + other->size_.length()) * 0.3f;
    const float overlap = combinedHalfSize - distance;

    if (overlap > 0) {
        const float separationDist = overlap * 0.5f + 0.1f;
        position_ += collisionDir * separationDist;
        other->position_ -= collisionDir * separationDist;
    }

    const threepp::Vector3 relativeVel = velocity_ - other->velocity_;
    const float velAlongNormal = relativeVel.dot(collisionDir);

    if (velAlongNormal > 0) return;
    constexpr float restitution = 0.6f;

    const float impulseMagnitude = -(1.0f + restitution) * velAlongNormal * 0.5f;
    const threepp::Vector3 impulse = collisionDir * impulseMagnitude;
    velocity_ += impulse;
    other->velocity_ -= impulse;
}

void Car::setSteeringLeftActive(const bool state) {
    isSteeringLeftActive = state;
    updateTargetSteering();
}

void Car::setSteeringRightActive(const bool state) {
    isSteeringRightActive = state;
    updateTargetSteering();
}

void Car::setAccelerationInput(const float amount) {
    accelerationInput = std::clamp(amount, -1.0f, 1.0f);
}

void Car::applyVelocity(const threepp::Vector3& vel) {
    velocity_ += vel;
}


void Car::setDragMultiplier(const float m) { dragMultiplier_ = m; }
void Car::setGripMultiplier(const float m) { gripMultiplier_ = m; }
void Car::setRollingResistanceMultiplier(const float m) { rollingResistanceMultiplier_ = m; }
void Car::setMaxSpeedMultiplier(const float m) { maxSpeedMultiplier_ = m; }

void Car::updateTargetSteering() {
    float target = 0.0f;
    if (isSteeringLeftActive && !isSteeringRightActive) target = -1.0f;
    else if (isSteeringRightActive && !isSteeringLeftActive) target = 1.0f;
    targetSteeringInput = std::clamp(target, -1.0f, 1.0f);
}

void Car::updateRotation(const float dt) {
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

void Car::updateVelocity(const float dt) {
    if (boostFactor_ > 1) boostFactor_ *= 0.994f;
    else if (boostFactor_ < 1) boostFactor_ *= 1.1f;

    const float forwardSpeed = velocity_.dot(forwardVector());
    float finalAccelerationMagnitude = 0;

    if (accelerationInput > 0.0f) {
        if (forwardSpeed < 0.0f) {
            finalAccelerationMagnitude = accelerationInput * brakeSpeed * boostFactor_;
        } else {
            const float throttleReduction = 1.0f - (forwardSpeed / (baseMaxSpeed * maxSpeedMultiplier_));
            finalAccelerationMagnitude = accelerationInput * accelerationSpeed * throttleReduction * boostFactor_;
        }
    } else if (accelerationInput < 0.0f) {
        if (forwardSpeed > 0.0f) {
            finalAccelerationMagnitude = accelerationInput * brakeSpeed;
        } else {
            finalAccelerationMagnitude = accelerationInput * accelerationSpeed;
        }
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

void Car::applySideFriction(const float dt) {
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

void Car::clampSpeed() {
    const float effectiveMaxSpeed = baseMaxSpeed * maxSpeedMultiplier_;
    const float speed = velocity_.length();
    if (speed > effectiveMaxSpeed) {
        threepp::Vector3 n = velocity_;
        n.normalize();
        velocity_ = n * effectiveMaxSpeed;
    }
}


CarSnap Car::snapshot() const {
    CarSnap snap;
    snap.size = size_;
    snap.position = position_;
    snap.prevPosition = prevPosition_;
    snap.forward = forwardVector();
    snap.rotation = rotation_;
    snap.speed = velocity_.length();
    snap.steering = currentSteeringInput;
    snap.boostFactor = boostFactor_;
    return snap;
}