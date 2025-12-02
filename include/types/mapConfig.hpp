#ifndef CARSIM_MAPCONFIG_HPP
#define CARSIM_MAPCONFIG_HPP

#include <vector>
#include "threepp/math/Vector3.hpp"

struct MapConfig {
    std::string name;
    std::string type;
    std::vector<threepp::Vector3> roadPoints;
    bool looped;
    int roadWidth;
    int width;
    int height;
    int checkpoints;
};

#endif //CARSIM_MAPCONFIG_HPP