#include "entities/Car.hpp"
#include <algorithm>
#include <iostream>
#include "types/gameSnapshots.hpp"

Car::Car(const threepp::Vector3& startPos, const float rotation)
      : ICollidable(CollisionType::Car, startPos, rotation) {
    accelerationSpeed_ = 400.f;
    brakeSpeed_ = 500.f;
    baseMaxSpeed_ = 750.f;
    rollingResistance_ = 0.5f;
    drag_ = 0.0002f;
    basegrip_ = 0.98;
    driftSlide_ = 0.85;
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

void Car::onCollision(const std::shared_ptr<ICollidable>& other, const float dt) {
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

void Car::resolveCollisionImpulse(const std::shared_ptr<Car>& other, const float dt) {
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
    velocity_ *= 0.995;
    velocity_ += impulse;
    other->velocity_ -= impulse;
}

void Car::setSteeringLeftActive(const bool state) {
    isSteeringLeftActive_ = state;
    updateTargetSteering();
}

void Car::setSteeringRightActive(const bool state) {
    isSteeringRightActive_ = state;
    updateTargetSteering();
}

void Car::setAccelerationInput(const float amount) {
    accelerationInput_ = std::clamp(amount, -1.0f, 1.0f);
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
    if (isSteeringLeftActive_ && !isSteeringRightActive_) target = -1.0f;
    else if (isSteeringRightActive_ && !isSteeringLeftActive_) target = 1.0f;
    targetSteeringInput_ = std::clamp(target, -1.0f, 1.0f);
}

void Car::updateRotation(const float dt) {
    const float speed = velocity_.length();
    const float step = steeringEaseRate_ * dt;

    if (currentSteeringInput_ < targetSteeringInput_)
        currentSteeringInput_ = std::min(currentSteeringInput_ + step, targetSteeringInput_);
    else if (currentSteeringInput_ > targetSteeringInput_)
        currentSteeringInput_ = std::max(currentSteeringInput_ - step, targetSteeringInput_);

    if (speed > 0.1f) {
        constexpr float maxRotationRate = 2.0f;
        const float speedFactor = maxRotationRate / (1.0f + speed / (baseMaxSpeed_ * maxSpeedMultiplier_));
        const threepp::Vector3 forward = forwardVector();
        const float dotProduct = velocity_.dot(forward);
        const float directionMultiplier = (dotProduct < 0.0f) ? -1.0f : 1.0f;
        rotation_ += currentSteeringInput_ * dt * speedFactor * directionMultiplier;
    }
}

void Car::updateVelocity(const float dt) {
    if (boostFactor_ > 1) boostFactor_ *= 0.994f;
    else if (boostFactor_ < 1) boostFactor_ *= 1.1f;

    const float forwardSpeed = velocity_.dot(forwardVector());
    float finalAccelerationMagnitude = 0;

    if (accelerationInput_ > 0.0f) {
        if (forwardSpeed < 0.0f) {
            finalAccelerationMagnitude = accelerationInput_ * brakeSpeed_ * boostFactor_;
        } else {
            const float throttleReduction = 1.0f - (forwardSpeed / (baseMaxSpeed_ * maxSpeedMultiplier_));
            finalAccelerationMagnitude = accelerationInput_ * accelerationSpeed_ * throttleReduction * boostFactor_;
        }
    } else if (accelerationInput_ < 0.0f) {
        if (forwardSpeed > 0.0f) {
            finalAccelerationMagnitude = accelerationInput_ * brakeSpeed_;
        } else {
            finalAccelerationMagnitude = accelerationInput_ * accelerationSpeed_;
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

    const float quadraticDrag = speed * speed * drag_ * dragMultiplier_;
    float linearDrag = 0.0f;
    if (accelerationInput_ <= 0.0f) {
        linearDrag = speed * rollingResistance_ * rollingResistanceMultiplier_;
    }
    const float totalDragMagnitude = quadraticDrag + linearDrag;
    const threepp::Vector3 dragForce = dragDirection * totalDragMagnitude;
    velocity_ -= dragForce * dt;
}

void Car::applySideFriction(const float dt) {
    if (gripMultiplier_ <= 0.0f) return;

    const auto forward = forwardVector();
    const float forwardSpeed = velocity_.dot(forward);
    auto lateral = velocity_ - forward * forwardSpeed;

    const float steeringMagnitude = std::abs(currentSteeringInput_);
    float dynamicFrictionFactor = basegrip_ - (basegrip_ - driftSlide_) * steeringMagnitude;

    dynamicFrictionFactor *= gripMultiplier_;
    dynamicFrictionFactor = std::clamp(dynamicFrictionFactor, 0.0f, 1.0f);

    lateral *= dynamicFrictionFactor;
    velocity_ = forward * forwardSpeed + lateral;
}

void Car::clampSpeed() {
    if (maxSpeedMultiplier_ <= 0.0f) return;
    const float effectiveMaxSpeed = baseMaxSpeed_ * maxSpeedMultiplier_;
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
    snap.steering = currentSteeringInput_;
    snap.boostFactor = boostFactor_;
    return snap;
}