#include "controllers/AIController.hpp"
#include <cmath>

AIController::AIController(const std::vector<RoadSample>& samples, const threepp::Vector3 pos, const float skill)
    : roadSamples_(samples),
      skillFactor_(skill),
      randomGenerator_(std::random_device{}()),
      randomDist_(0.0f, 1.0f), currOffset_(0) {
    targetWaypointIndex_ = findNearestSample(samples, pos);
}

void AIController::calculateNewOffset() {
    const RoadSample targetSample = roadSamples_[targetWaypointIndex_];
    const float maxOffset = targetSample.width * 0.9f;
    const float noise = (randomDist_(randomGenerator_) * 2.0f) - 1.0f;
    currOffset_ = noise * (1.0f - skillFactor_) * maxOffset;
}

size_t AIController::findNearestSample(const std::vector<RoadSample>& samples, const threepp::Vector3& pos) {
    size_t nearest = 0;
    float minDist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < samples.size(); ++i) {
        const float d = pos.distanceTo(samples[i].pos);
        if (d < minDist) { minDist = d; nearest = i; }
    }
    return nearest;
}

void AIController::update(float dt, Car& car) {
    if (roadSamples_.empty()) return;

    const threepp::Vector3 carPos = car.getPosition();
    const threepp::Vector3 currentTargetPos = roadSamples_[targetWaypointIndex_].pos;

    if (carPos.distanceTo(currentTargetPos) < waypointTolerance_ + 50 * skillFactor_) {
        targetWaypointIndex_ += static_cast<size_t>(5 + 20 * skillFactor_);
        if (targetWaypointIndex_ >= roadSamples_.size()) {
            targetWaypointIndex_ %= roadSamples_.size();
        }
        calculateNewOffset();
    }

    const RoadSample& targetSample = roadSamples_[targetWaypointIndex_];

    const threepp::Vector3 finalTargetPos = targetSample.pos + (targetSample.right * currOffset_);

    threepp::Vector3 toTarget = finalTargetPos - carPos;
    toTarget.normalize();

    const threepp::Vector3 carForward = car.forwardVector();
    const float baseSteeringInput = carForward.x * toTarget.y - carForward.y * toTarget.x;

    car.setSteeringLeftActive(false);
    car.setSteeringRightActive(false);

    constexpr float baseDeadZone = 0.05f;
    if (baseSteeringInput < -baseDeadZone) {
        car.setSteeringRightActive(true);
    } else if (baseSteeringInput > baseDeadZone) {
        car.setSteeringLeftActive(true);
    }

    const float turnSharpness = std::abs(baseSteeringInput);
    const float corneringFactor = 1.0f - (turnSharpness * (0.5f + (0.9f * skillFactor_)));

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