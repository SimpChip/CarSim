//
// Created by Mella on 13.11.2025.
//

#ifndef CARSIM_ICONTROLER_HPP
#define CARSIM_ICONTROLER_HPP

#include <CarLogic.hpp>
#include <random>

class IController {
public:
    virtual void update(float dt, CarLogic& car) = 0;
    virtual ~IController() = default;
};

class PlayerController final : public IController {

public:
    CarLogic& car_;
    explicit PlayerController(CarLogic& car) : car_(car) {
        std::cout << "Player controller created for car" << std::endl;
    }

    void onKeyPressed(const threepp::KeyEvent evt) const {;
        switch(evt.key) {
            case threepp::Key::Q: car_.setBoostFactor(2); break;
            case threepp::Key::W: car_.setAccelerationInput(1.0f); break;
            case threepp::Key::S: car_.setAccelerationInput(-1.0f); break;
            case threepp::Key::A: car_.setSteeringLeftActive(true); break;
            case threepp::Key::D: car_.setSteeringRightActive(true); break;
            default: ;
        }
    }

    void onKeyReleased(const threepp::KeyEvent evt) const {
        switch(evt.key) {
            case threepp::Key::W:
            case threepp::Key::S: car_.setAccelerationInput(0.0f); break;
            case threepp::Key::A: car_.setSteeringLeftActive(false); break;
            case threepp::Key::D: car_.setSteeringRightActive(false); break;
            default: ;
        }
    }

    void update(float dt, CarLogic& car) override {
    }
};

class AIController final : public IController {
public:
    const std::vector<RoadSample>& roadSamples_;
    size_t targetWaypointIndex_ = 0;
    const float WAYPOINT_TOLERANCE = 175.0f;

    const float skillFactor_;

    // Randomization members
    std::mt19937 randomGenerator_;
    std::uniform_real_distribution<> randomDist_;

    // New: Store the offset persistently so the car commits to a "line"
    // instead of vibrating left/right every frame
    float currentLateralOffset_ = 0.0f;

    explicit AIController(const std::vector<RoadSample>& samples, const float startIndex, const float skill):
        roadSamples_(samples),
        targetWaypointIndex_(startIndex + 1),
        skillFactor_(skill),
        // Initialize random generator once
        randomGenerator_(std::random_device{}()),
        randomDist_(0.0f, 1.0f)
    {
        calculateNewOffset(); // Calculate initial driving line
    }

    void calculateNewOffset() {
        const RoadSample& targetSample = roadSamples_[targetWaypointIndex_];
        const float MAX_LATERAL_OFFSET = targetSample.width * 0.65f;
        float noise = (randomDist_(randomGenerator_) * 2.0f) - 1.0f; // Range -1 to 1
        currentLateralOffset_ = noise * (1.0f - skillFactor_) * MAX_LATERAL_OFFSET;
    }

    void update(float dt, CarLogic& car) override {
        if (roadSamples_.empty()) return;

        const Vector3 carPos = car.getPosition();
        const Vector3 currentTargetPos = roadSamples_[targetWaypointIndex_].pos;

        if (carPos.distanceTo(currentTargetPos) < WAYPOINT_TOLERANCE + 50 * skillFactor_) {
            targetWaypointIndex_ += 5 + 10 * skillFactor_;
            if (targetWaypointIndex_ >= roadSamples_.size()) {
                targetWaypointIndex_ = targetWaypointIndex_ % roadSamples_.size();
            }
            calculateNewOffset();
        }

        const RoadSample& targetSample = roadSamples_[targetWaypointIndex_];

        const Vector3 finalTargetPos = targetSample.pos + (targetSample.right * currentLateralOffset_);

        Vector3 toTarget = finalTargetPos - carPos;
        toTarget.normalize();

        const Vector3 carForward = car.forwardVector();

        const float baseSteeringInput = carForward.x * toTarget.y - carForward.y * toTarget.x;

        constexpr float BASE_DEADZONE = 0.05f;
        car.setSteeringLeftActive(false);
        car.setSteeringRightActive(false);

        if (baseSteeringInput < -BASE_DEADZONE) {
            car.setSteeringRightActive(true);
        } else if (baseSteeringInput > BASE_DEADZONE) {
            car.setSteeringLeftActive(true);
        }

        float turnSharpness = std::abs(baseSteeringInput);

        float corneringFactor = 1.0f - (turnSharpness * (0.5f + (0.9f * skillFactor_)));

        if (skillFactor_ < 0.4f) {
             corneringFactor += 0.2f;
        }


        float accelInput = 0.0f;

        if (corneringFactor > 0.6f) {
            accelInput = 1.0f;
        }
        else if (corneringFactor < 0.3f) {
            accelInput = -1.0f;
        }
        else {
            accelInput = 0.0f;
        }

        if (car.getVelocity().length() < 20.0f) {
            accelInput = 1.0f;
        }

        car.setAccelerationInput(accelInput);
    }
};

#endif //CARSIM_ICONTROLER_HPP