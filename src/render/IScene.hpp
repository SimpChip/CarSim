//
// Created by Mella on 25.11.2025.
//

#ifndef CARSIM_ISCENE_HPP
#define CARSIM_ISCENE_HPP

#include "threepp/threepp.hpp"

class IScene {
public:
    virtual ~IScene() = default;
    virtual void render(threepp::GLRenderer& renderer) = 0;
};

#endif //CARSIM_ISCENE_HPP