#ifndef CARSIM_SNAPSHOTS_HPP
#define CARSIM_SNAPSHOTS_HPP

#include "vector"
#include "threepp/math/Vector3.hpp"
#include "data.hpp"

struct PickupSnap {
    PickupType type;
    threepp::Vector3 position;
    float rotation;
    float radius;
    bool active = true;
    float respawnTimer = 0.0f;
};

struct CarSnap {
    threepp::Vector3 size;
    threepp::Vector3 position;
    threepp::Vector3 prevPosition;
    threepp::Vector3 forward;
    float rotation;
    float speed;
    float steering;
    float boostFactor;
};

struct CpSnap {
    threepp::Vector3 position;
    threepp::Vector3 forward;
    float width;
    bool start;
};

struct MapSysSnap {
    std::string name;
    std::vector<RoadSample> roadSamples;
    float roadWidth;
    int width;
    int height;
    bool looped;
};

struct LapSysSnap {
    float raceTimer;
    float startTimer;
    int startIndex;
    std::vector<CpSnap> checkpoints;
    std::unordered_map<int, LapTracker> laptrackers;
};

struct GameSnap {
    float scale;
    int startIdx;
    int playerCarId;
    MapSysSnap map;
    LapSysSnap lap;
    std::vector<CarSnap> cars;
    std::vector<PickupSnap> pickups;
};






#endif //CARSIM_SNAPSHOTS_HPP