#ifndef CARSIM_AICONTROLLER_HPP
#define CARSIM_AICONTROLLER_HPP
#include "IController.hpp"
#include "../entities/Car.hpp"
#include <random>
#include <vector>

class AIController final : public IController {
public:
    explicit AIController(const std::vector<RoadSample>& samples, const threepp::Vector3 pos, float skill);

    void update(float dt, Car& car) override;
private:
    const std::vector<RoadSample>& roadSamples_;

    float waypointTolerance_ = 175.0f;
    size_t targetWaypointIndex_;
    const float skillFactor_;
    std::mt19937 randomGenerator_;
    std::uniform_real_distribution<> randomDist_;
    float currOffset_;

    void calculateNewOffset();

    static size_t findNearestSample(const std::vector<RoadSample>& samples, const threepp::Vector3& pos);
};

#endif // CARSIM_AICONTROLLER_HPP
