//
// Created by Mella on 25.11.2025.
//

#ifndef CARSIM_GAMESTATE_HPP
#define CARSIM_GAMESTATE_HPP

#include "threepp/math/Vector3.hpp"
#include "vector"

struct RoadSample {
    threepp::Vector3 pos;
    threepp::Vector3 tangent;
    threepp::Vector3 right;
    float width;
};

enum class PickupType {
    Coin,
    BadCoin,
};

enum class CollisionType { Car, SpeedBoost, SlowBoost, Obstacle };

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
    bool active = true;
    float radius = 6.0f;
    float respawnTimer = 0.0f;
};

// --- Checkpoint ---
struct Checkpoint {
    int id;
    threepp::Vector3 position;
    threepp::Vector3 forward;
    float width{};
    float depth{};
    bool active = true;
    bool passed = false;
};

struct CheckpointData {
    Checkpoint startpoint;
    std::vector<Checkpoint> checkpoints;
    bool allCheckPassed;
    int startCheckpointIndex;
    int currentCheckpointIndex;

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

// --- Game ---
struct GameState {
    float scale;
    std::vector<CarData> cars;
    MapData mapData;
    CheckpointData checkpointsData;
    std::vector<PickupData> pickups;
    float lapTimer;
    float lastLapTime;

};

// --- Map Creator ---
struct MapCreatorState {
    std::vector<threepp::Vector3> points;
    int mapSize;
    bool looped;
};


#endif //CARSIM_GAMESTATE_HPP