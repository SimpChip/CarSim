//
// Created by Mella on 18.10.2025.
//

#ifndef CARSIM_GAMESCENE_HPP
#define CARSIM_GAMESCENE_HPP
#include <threepp/threepp.hpp>
#include <iostream>
#include <cmath>

#include "RaceGame.hpp"

class GameScene
{
    public:
        std::shared_ptr<threepp::Scene> scene;
        std::shared_ptr<threepp::PerspectiveCamera> camera;
        GameScene(threepp::Vector3& carPos, float& carRotation, RaceGame& raceGame);
        void update(const threepp::Vector3& carPos, const float& carRotation, const float& dt) const;

    private:
        std::shared_ptr<threepp::Group> carMesh;
        std::shared_ptr<threepp::Mesh> colBox;

};


#endif //CARSIM_GAMESCENE_HPP