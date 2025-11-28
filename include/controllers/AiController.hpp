#ifndef CARSIM_AICONTROLLER_HPP
#define CARSIM_AICONTROLLER_HPP

#include "IController.hpp"
#include "logic/CarLogic.hpp"
#include <random>
#include <vector>

class AIController final : public IController {
public:
    explicit AIController(const std::vector<RoadSample>& samples, size_t startIndex, float skill);
    void update(float dt, CarLogic& car) override;
private:

    static constexpr float WAYPOINT_TOLERANCE = 175.0f;

    const std::vector<RoadSample>& roadSamples_;
    size_t targetWaypointIndex_;
    const float skillFactor_;

    std::mt19937 randomGenerator_;
    std::uniform_real_distribution<> randomDist_;
    float currentLateralOffset_ = 0.0f;

    void calculateNewOffset();
};

#endif // CARSIM_AICONTROLLER_HPP
