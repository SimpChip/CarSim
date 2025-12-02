#ifndef CARSIM_MAPMAKERSNAPSHOTS_HPP
#define CARSIM_MAPMAKERSNAPSHOTS_HPP

#include <vector>
#include "threepp/math/Vector3.hpp"

struct MapMakerSnap {
    std::vector<threepp::Vector3> points;
    bool looped;
    int mapSize;
};

#endif //CARSIM_MAPMAKERSNAPSHOTS_HPP