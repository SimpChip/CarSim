//
// Created by Mella on 23.10.2025.
//

#ifndef CARSIM_CARCONFIG_HPP
#define CARSIM_CARCONFIG_HPP


#pragma once

#include <string>
#include <vector>
#include <memory>
// Assuming these threepp headers are available in your project structure
#include <threepp/math/Vector3.hpp>
#include <threepp/objects/Group.hpp>
#include <threepp/math/Color.hpp>

// Forward declaration needed for the dynamic target assignment
struct WheelDef {
    std::string filename;
    threepp::Vector3 pos;
    std::shared_ptr<threepp::Group>* target = nullptr;
};

struct CarMaterialDef {
    threepp::Color baseColor;
};

struct CarModelConfig {
    std::string modelDirectoryName;

    float modelScaleFactor;
    float modelWidth;
    float modelHeight;
    float modelLength;

    std::vector<WheelDef> wheelDefs;
    CarMaterialDef materialDef;
};

#endif //CARSIM_CARCONFIG_HPP