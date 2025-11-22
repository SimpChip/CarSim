//
// Created by Mella on 19.11.2025.
//

#ifndef CARSIM_TYPES_H
#define CARSIM_TYPES_H
#include "threepp/threepp.hpp"

struct ImageInfo {
    int width, height, channels;
    std::string name;
    unsigned char *data;
};

struct RoadSample {
    Vector3 pos;       // centerline position
    Vector3 tangent;   // direction along road
    Vector3 right;     // perpendicular vector
    float width;       // local road width
};

enum class PickupType {
    Coin,
};

struct Pickup {
    PickupType type;
    threepp::Vector3 position;
    bool active = true;
    float radius = 6.0f;
    float respawnTimer = 0.0f;
};

struct Checkpoint {
    int id;
    threepp::Vector3 position;
    threepp::Vector3 forward;
    float width{};
    float depth{};
    bool active = true;
    bool passed = false;
};

#endif //CARSIM_TYPES_H