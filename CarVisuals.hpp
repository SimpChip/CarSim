//
// Created by Mella on 29.10.2025.
//

#ifndef CARSIM_CARVISUALS_HPP
#define CARSIM_CARVISUALS_HPP

#pragma once
#include <iostream>
#include <threepp/threepp.hpp>
#include <utility>

#include "CarLogic.hpp"


class CarVisuals {
public:

    CarVisuals(const float scale, std::string objectName) : scale_(scale), objectName_(std::move(objectName))  {

        threepp::OBJLoader ol;

        const std::string base = std::string(DATA_DIR) + "/models/" + objectName_;
        if (const std::string carPath = base + "/car.obj"; std::filesystem::exists(carPath)) {
            try {
                carMesh = ol.load(carPath, true);
                carMesh->scale = Vector3(scale_*1.f, scale_*1.f, scale_*1.f);
            } catch (...) {
                carMesh = createPlaceholder(scale_, 2.f, 4.f, 1.f);
                carMesh->rotation.x -threepp::math::PI / 2.f;
            }
        } else {
            carMesh = createPlaceholder(scale_, 2.f, 4.f, 1.f);
            carMesh->rotation.x -threepp::math::PI / 2.f;
        }

        carMesh->position = Vector3(0, 0, 0);

        //crazycar = 1.05f, 0.4f,  1.4f
        //camping =  1 , 0.5, 1.05
        //redcar = 0.9 , 0.4, 1.3

        struct WheelDef { std::string filename; threepp::Vector3 pos; std::shared_ptr<threepp::Group>* target; };
        std::vector<WheelDef> wheelDefs = {
            { "wheel_fl.obj", threepp::Vector3( 1.f, 1.05f,  0.5f), &wheelFrontLMesh },
            { "wheel_fr.obj", threepp::Vector3(-1.f, 1.05f,  0.5f), &wheelFrontRMesh },
            { "wheel_bl.obj", threepp::Vector3( 1.f, -1.05f, 0.5f), &wheelBackLMesh  },
            { "wheel_br.obj", threepp::Vector3(-1.f, -1.05f, 0.5f), &wheelBackRMesh  },
        };

        wheels.clear(); // member vector
        for (auto &wd : wheelDefs) {
            const std::string wpath = base + "/" + wd.filename;
            std::shared_ptr<threepp::Group> loadedWheel = nullptr;

            if (std::filesystem::exists(wpath)) {
                try {
                    loadedWheel = ol.load(wpath, true); // ol.load accepts std::string
                } catch (...) {
                    loadedWheel = createPlaceholder(scale_, 0.5f,0.5f,0.5f);
                }
            } else {
                std::cout << wd.filename << "is not implmented"<<std::endl;
            }
            if (loadedWheel) {
                auto steeringPivot = threepp::Group::create();
                steeringPivot->position = wd.pos;
                steeringPivot->add(loadedWheel);
                carMesh->add(steeringPivot);
                *(wd.target) = steeringPivot;
                wheels.push_back(steeringPivot);
            } else {
                *(wd.target) = nullptr;
            }
        }
    }

    std::shared_ptr<threepp::Group> getCarMesh() {
        return carMesh;
    }


    void update(CarLogic& carLogic, float& dt) {

        carMesh->position = carLogic.getPosition();
        carMesh->rotation.z = -carLogic.getRotation();

        auto applySteer = [&](const std::shared_ptr<threepp::Group> &wheel, float steerAngle) {
            if (!wheel) return;
            wheel->rotation.z = steerAngle;
        };

        float wheelRadius = scale_;
        float rollAmount = carLogic.velocity_.length() * dt / wheelRadius;
        auto applyWheelRoll = [&](const std::shared_ptr<threepp::Group> &wheelPivot, float sign = 1.0f) {
            if (!wheelPivot) return;
            if (wheelPivot->children.empty()) return;

            auto wheelMesh = wheelPivot->children.front();
            wheelMesh->rotation.x += rollAmount * sign;
        };

        float steeringYawFactor = 0.5f;
        float steer = carLogic.getCurrentSteeringInput() * steeringYawFactor;
        applySteer(wheelFrontRMesh,  -steer);
        applySteer(wheelFrontLMesh,  -steer);

        applyWheelRoll(wheelFrontRMesh, +1.0f);
        applyWheelRoll(wheelFrontLMesh, +1.0f);
        applyWheelRoll(wheelBackRMesh,  +1.0f);
        applyWheelRoll(wheelBackLMesh,  +1.0f);
    }

private:
    std::shared_ptr<threepp::Group> carMesh;
    std::string objectName_;
    std::shared_ptr<threepp::Group> carBodyMesh;
    std::shared_ptr<threepp::Group> wheelFrontLMesh, wheelFrontRMesh, wheelBackLMesh, wheelBackRMesh;
    std::vector<std::shared_ptr<threepp::Group>> wheels;
    float scale_;

    static std::shared_ptr<threepp::Group> createPlaceholder(float scale, float x, float y, float z)
    {
        auto group = threepp::Group::create();
        auto geom = threepp::BoxGeometry::create(x * scale, y * scale, z * scale);
        auto mat  = threepp::MeshStandardMaterial::create();
        mat->color = threepp::Color::rebeccapurple;
        auto mesh = threepp::Mesh::create(geom, mat);
        mesh->position.set(0, 0.5f * scale, 0);
        group->add(mesh);
        return group;
    }

};

#endif //CARSIM_CARVISUALS_HPP