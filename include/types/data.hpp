//
// Created by Mella on 27.11.2025.
//

#ifndef CARSIM_DATA_HPP
#define CARSIM_DATA_HPP

#include <unordered_map>
#include "threepp/math/Vector3.hpp"
#include "vector"


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
    float width;
};

struct LapTracker {
    int carId;
    int currentLap{0};
    int currentCheckpoint{};
    int nextCheckpoint{0};
    bool lapCompleted{false};
    bool firstLap{false};
    float lapStartTime{0.f};
    float lastLapTime{0.f};
};



enum class PickupType {
    Coin,
    BadCoin,
};

enum class CollisionType { Car, Checkpoint, SpeedBoost, SlowBoost, Obstacle };

struct CollisionEnterEvent {
    int idA, idB;
    CollisionType typeA, typeB;
};
struct CollisionExitEvent {
    int idA, idB;
    CollisionType typeA, typeB;
};

// --- Car ---
struct CarData {
    threepp::Vector3 carSize;
    threepp::Vector3 position;
    threepp::Vector3 previousPosition;
    threepp::Vector3 forward;
    float rotation;
    float speed;
    float steering;
    float boostFactor;
};

// --- Pickup ---
struct PickupData {
    PickupType type;
    threepp::Vector3 position;
    float rotation;
    float radius;
    bool active = true;
    float respawnTimer = 0.0f;
};

// --- Checkpoint ---
struct CheckpointData {
    threepp::Vector3 position;
    threepp::Vector3 forward;
    float width;
    bool start;
};


// --- Map ---
struct MapData {
    std::string mapName;
    std::vector<RoadSample> roadSamples;
    float roadWidth;
    int width;
    int height;
    bool looped;
};

struct LapManagerData {
    float lapTimer;
    std::vector<CheckpointData> checkpoints;
    std::unordered_map<int, LapTracker> laptrackers;
};

// --- Game ---
struct GameState {
    float scale;
    std::vector<CarData> cars;
    MapData mapData;
    std::vector<PickupData> pickups;
    LapManagerData lapManagerData;
    int playerCarId;
};

// --- Map Creator ---
struct MapCreatorState {
    std::vector<threepp::Vector3> points;
    int mapSize;
    bool looped;
};

#endif //CARSIM_DATA_HPP