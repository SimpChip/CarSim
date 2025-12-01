#include <algorithm>

#include "controllers/AIController.hpp"
#include <cmath>
#include <iostream>

AIController::AIController(const std::vector<RoadSample>& samples, const threepp::Vector3 pos, const float skill)
    : roadSamples_(samples),
      skillFactor_(skill),
      randomGenerator_(std::random_device{}()),
      randomDist_(0.0f, 1.0f) {
    targetWaypointIndex_ = findNearestSample(samples, pos);
}

void AIController::calculateNewOffset() {
    const RoadSample targetSample = roadSamples_[targetWaypointIndex_];
    const float MAX_LATERAL_OFFSET = targetSample.width * 0.9f;
    const float noise = (randomDist_(randomGenerator_) * 2.0f) - 1.0f; // -1 to 1
    currentLateralOffset_ = noise * (1.0f - skillFactor_) * MAX_LATERAL_OFFSET;
}

size_t AIController::findNearestSample(const std::vector<RoadSample>& samples, const threepp::Vector3& pos) const {
    size_t nearest = 0;
    float minDist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < samples.size(); ++i) {
        float d = pos.distanceTo(samples[i].pos);
        if (d < minDist) { minDist = d; nearest = i; }
    }
    return nearest;
}

void AIController::update(float dt, Car& car) {
    if (roadSamples_.empty()) return;

    const threepp::Vector3 carPos = car.getPosition();
    const threepp::Vector3 currentTargetPos = roadSamples_[targetWaypointIndex_].pos;

    if (carPos.distanceTo(currentTargetPos) < WAYPOINT_TOLERANCE + 50 * skillFactor_) {
        targetWaypointIndex_ += static_cast<size_t>(5 + 10 * skillFactor_);
        if (targetWaypointIndex_ >= roadSamples_.size()) {
            targetWaypointIndex_ %= roadSamples_.size();
        }
        calculateNewOffset();
    }

    const RoadSample& targetSample = roadSamples_[targetWaypointIndex_];
    const threepp::Vector3 finalTargetPos = targetSample.pos + (targetSample.right * currentLateralOffset_);

    threepp::Vector3 toTarget = finalTargetPos - carPos;
    toTarget.normalize();

    const threepp::Vector3 carForward = car.forwardVector();
    const float baseSteeringInput = carForward.x * toTarget.y - carForward.y * toTarget.x;

    car.setSteeringLeftActive(false);
    car.setSteeringRightActive(false);

    constexpr float BASE_DEADZONE = 0.05f;
    if (baseSteeringInput < -BASE_DEADZONE) {
        car.setSteeringRightActive(true);
    } else if (baseSteeringInput > BASE_DEADZONE) {
        car.setSteeringLeftActive(true);
    }

    const float turnSharpness = std::abs(baseSteeringInput);
    float corneringFactor = 1.0f - (turnSharpness * (0.5f + (0.9f * skillFactor_)));

    if (skillFactor_ < 0.4f) {
        corneringFactor += 0.2f;
    }

    float accelInput = 0.0f;
    if (corneringFactor > 0.6f) {
        accelInput = 1.0f;
    } else if (corneringFactor < 0.3f) {
        accelInput = -1.0f;
    }
    if (car.getVelocity().length() < 20.0f) {
        accelInput = 1.0f;
    }

    car.setAccelerationInput(accelInput);
}