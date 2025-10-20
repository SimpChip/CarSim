//
// Created by Mella on 18.10.2025.
//

#include "CarLogic.hpp"
#include "iostream"
#include <cmath>

CarLogic::CarLogic(Vector3 startPos, float rotation)
    : position(startPos), previousPosition(startPos), rotation(rotation),
      accelerationInput(0.0f), steeringInput(0.0f),
      maxSpeed(100.0f), accelerationSpeed(150.0f),
      maxTurnRate(2.0f), sideFriction(0), drag(0.95f)
{}

Vector3& CarLogic::getPosition() const { return position; }
Vector3 CarLogic::getPreviousPosition() const { return previousPosition; }
float& CarLogic::getRotation() const { return rotation; }
Vector3 CarLogic::getVelocity() const { return velocity; }

void CarLogic::setPosition(const Vector3& pos) { position = pos; }
void CarLogic::setVelocity(const Vector3& vel) { velocity = vel; }

void CarLogic::setMaxSpeed(float speed) { maxSpeed = speed; }
void CarLogic::setAccelerationSpeed(float accel) { accelerationSpeed = accel; }
void CarLogic::setMaxTurnRate(float rate) { maxTurnRate = rate; }
void CarLogic::setSideFriction(float friction) { sideFriction = friction; }

void CarLogic::setAccelerationInput(float amount) { accelerationInput = std::clamp(amount, -1.0f, 1.0f); }
void CarLogic::setSteeringInput(float amount) { steeringInput = std::clamp(amount, -1.0f, 1.0f); }
void CarLogic::applyBoost(float boostAmount) { velocity += forwardVector() * boostAmount; }

void CarLogic::stopCar() { velocity = Vector3(0,0,0); }

void CarLogic::update(float dt)
{
    previousPosition = position;
    updateRotation(dt);
    updateVelocity(dt);
    applySideFriction();
    clampSpeed();
    position += velocity * dt;
}

void CarLogic::applyBraking(float factor)
{
    velocity *= factor;
}

Vector3 CarLogic::forwardVector() const
{
    return Vector3(std::sin(rotation), 0, std::cos(rotation));
}

void CarLogic::updateRotation(float dt)
{
    float speed = velocity.length();
    float speedFactor = std::min(speed / maxSpeed, 1.0f);
    rotation += steeringInput * maxTurnRate * dt * (1.0f - 0.5f * speedFactor);
}

void CarLogic::updateVelocity(float dt)
{
    velocity += forwardVector() * (accelerationInput * accelerationSpeed * dt);
    if (velocity.length() < 0.1f)
    {
        velocity = Vector3(0,0,0);
    }

    velocity *= drag;
}

void CarLogic::applySideFriction()
{
    Vector3 forward = forwardVector();
    float forwardSpeed = velocity.dot(forward);
    Vector3 lateral = velocity - forward * forwardSpeed;
    lateral *= sideFriction;
    velocity = forward * forwardSpeed + lateral;
}

void CarLogic::clampSpeed()
{
    float speed = velocity.length();
    if (speed > maxSpeed)
        velocity = velocity.normalize() * maxSpeed;
}