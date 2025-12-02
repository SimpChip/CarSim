
#ifndef CARSIM_DATA_HPP
#define CARSIM_DATA_HPP

#include <array>
#include <unordered_map>
#include "threepp/math/Vector3.hpp"
#include "threepp/math/Vector2.hpp"
#include "vector"
#include "types.hpp"

struct PairHash {
    std::size_t operator()(const std::pair<int,int>& p) const noexcept {
        const auto h1 = std::hash<int>{}(p.first);
        const auto h2 = std::hash<int>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};




struct RoadSample {
    threepp::Vector3 pos;
    threepp::Vector3 tangent;
    threepp::Vector3 right;
    int width;
};

struct LapTracker {
    int carId{};
    int currentCheckpoint{};
    int currentLap{0};
    int nextCheckpoint{0};
    bool lapCompleted{false};
    float currentLapTime{0.f};
    float lapStartTime{0.f};
    float lastLapTime{0.f};
};

struct CarData {
    threepp::Vector3 carSize;
    threepp::Vector3 position;
    threepp::Vector3 previousPosition;
    threepp::Vector3 forward;
    float rotation;
    float speed;
    float steering;
    float boostFactor;
    std::array<threepp::Vector2, 4> corners;
};

struct PickupData {
    PickupType type;
    threepp::Vector3 position;
    float rotation;
    float radius;
    bool active = true;
    float respawnTimer = 0.0f;
};

struct CheckpointData {
    threepp::Vector3 position;
    threepp::Vector3 forward;
    float width;
    bool start;
    std::array<threepp::Vector2,4> corners;
};

struct MapData {
    std::string mapName;
    std::vector<RoadSample> roadSamples;
    float roadWidth;
    int width;
    int height;
    bool looped;
};

struct LapManagerData {
    float raceTimer;
    float startTimer;
    int startIndex;
    std::vector<CheckpointData> checkpoints;
    std::unordered_map<int, LapTracker> laptrackers;
};

#endif //CARSIM_DATA_HPP