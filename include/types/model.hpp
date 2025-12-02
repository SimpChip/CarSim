#ifndef CARSIM_MODEL_HPP
#define CARSIM_MODEL_HPP
#include <string>
#include <unordered_map>
#include <vector>
#include "threepp/objects/Group.hpp"

struct ModelVariant {
    std::string color;
    std::shared_ptr<threepp::Group> body;
};

struct ModelData {
    std::shared_ptr<threepp::Group> body;
    std::vector<std::shared_ptr<threepp::Group>> wheels;
};

struct ModelEntry {
    std::string name;
    std::vector<std::string> wheelPaths;
    std::unordered_map<std::string, std::string> colorBodies;
};

#endif //CARSIM_MODEL_HPP