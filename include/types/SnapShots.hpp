//
// Created by Mella on 29.11.2025.
//

#ifndef CARSIM_SNAPSHOTS_HPP
#define CARSIM_SNAPSHOTS_HPP

#include "ILogicSnapShot.hpp"
#include "vector"
#include "threepp/math/Vector3.hpp"
#include "data.hpp"

struct MapMakerSnapshot {
    std::vector<threepp::Vector3> points;
    bool looped = false;
    int mapSize;
};

struct GameSnapShot {
    float scale;
    int startIdx;
    std::vector<CarData> cars;
    MapData mapData;
    std::vector<PickupData> pickups;
    LapManagerData lapManagerData;
    int playerCarId;
};

#endif //CARSIM_SNAPSHOTS_HPP