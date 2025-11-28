//
// Created by Mella on 29.10.2025.
//

#ifndef CARSIM_CARVISUALS_HPP
#define CARSIM_CARVISUALS_HPP

#include <iostream>
#include "core/ResourceManager.hpp"
#include "threepp/geometries/BoxGeometry.hpp"
#include "threepp/materials/MeshStandardMaterial.hpp"
#include "threepp/math/MathUtils.hpp"
#include "threepp/math/Vector3.hpp"
#include "threepp/objects/Group.hpp"
#include "threepp/objects/Mesh.hpp"


class CarVisuals {
public:
    CarVisuals(
        std::string objectName,
        const threepp::Vector3 carStartPos,
        const float carRot,
        const threepp::Vector3 carSize,
        const float scale,
        ResourceManager& resources,
        const std::string &color = "basic")
    :
        objectName_(std::move(objectName)), scale_(scale)
    {
        const std::string base = std::string(DATA_DIR) + "/models/" + objectName_;
        const std::string carPath = base +"/"+color+"/car.obj";
        if (std::filesystem::exists(carPath)) {
            try {
                const auto proto = resources.loadPrototype(carPath);
                if (!proto) throw std::runtime_error("prototype load returned null");

                carMesh = std::dynamic_pointer_cast<threepp::Group>(proto->clone(true));
                if (!carMesh) throw std::runtime_error("clone failed");

                threepp::Box3 bounds;
                bounds.setFromObject(*carMesh);

                threepp::Vector3 size = bounds.getSize();

                float scaleX = carSize.x  / size.x;
                float scaleY = carSize.y / size.y;
                float scaleZ = carSize.z / size.z;

                std::cout << scaleZ << "-" << scaleY << "-" << scaleX << std::endl;

                carMesh->scale = threepp::Vector3(scaleX,scaleY,scaleZ);

                std::cout << "Car " << " size: "
                  << carSize.x << ", "
                  << carSize.y << ", "
                  << carSize.z << "\n";
                std::cout << "Car mesh loaded and cloned for instance\n";

            } catch (const std::exception& ex) {
                std::cerr << "Failed to load or clone car model: " << ex.what() << "\n";
                carMesh = createPlaceholder(scale_, 2.f, 4.f, 1.f);
                carMesh->rotation.x = -threepp::math::PI / 2.f;
            } catch (...) {
                std::cerr << "Unknown error loading car model\n";
                carMesh = createPlaceholder(scale_, 2.f, 4.f, 1.f);
                carMesh->rotation.x = -threepp::math::PI / 2.f;
            }
        } else {
            std::cout << "Car model file does not exist: " << carPath << "\n";
            carMesh = createPlaceholder(scale_, 2.f, 4.f, 1.f);
        }

        carMesh->position = carStartPos;
        carMesh->name = objectName;

        //crazycar = 1.05f, 0.4f,  1.4f
        //camping =  1 , 0.5, 1.05
        //redcar = 0.9 , 0.4, 1.3

        struct WheelDef { std::string filename; threepp::Vector3 pos; std::shared_ptr<threepp::Group>* target; };
        const std::vector<WheelDef> wheelDefs = {
            { "wheel_fl.obj", threepp::Vector3( 1.f, 1.05f,  0.5f), &wheelFrontLMesh },
            { "wheel_fr.obj", threepp::Vector3(-1.f, 1.05f,  0.5f), &wheelFrontRMesh },
            { "wheel_bl.obj", threepp::Vector3( 1.f, -1.05f, 0.5f), &wheelBackLMesh  },
            { "wheel_br.obj", threepp::Vector3(-1.f, -1.05f, 0.5f), &wheelBackRMesh  },
        };

        wheels.clear();
        for (const auto &[filename, pos, target] : wheelDefs) {
            const std::string wpath = base + "/"+filename;
            std::shared_ptr<threepp::Group> protoWheel = nullptr;

            if (std::filesystem::exists(wpath)) {
                try {
                    protoWheel = resources.loadPrototype(wpath);
                    if (!protoWheel) {
                        throw std::runtime_error("prototype null");
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Error loading wheel prototype " << filename << ": " << e.what() << "\n";
                    protoWheel = createPlaceholder(scale_, 0.5f, 0.5f, 0.5f);
                } catch (...) {
                    std::cerr << "Unknown error loading wheel prototype " << filename << "\n";
                    protoWheel = createPlaceholder(scale_, 0.5f, 0.5f, 0.5f);
                }
            } else {
                std::cout << filename << " is not implemented\n";
            }

            if (protoWheel) {
                auto loadedWheel = std::dynamic_pointer_cast<threepp::Group>(protoWheel->clone(true));
                if (!loadedWheel) {
                    loadedWheel = createPlaceholder(scale_, 0.5f, 0.5f, 0.5f);
                }

                auto steeringPivot = threepp::Group::create();
                steeringPivot->position = pos;
                steeringPivot->add(loadedWheel);
                carMesh->add(steeringPivot);
                *(target) = steeringPivot;
                wheels.push_back(steeringPivot);
            } else {
                *(target) = nullptr;
            }
        }
    }

    std::shared_ptr<threepp::Group> getCarMesh() {
        return carMesh;
    }

    void update(const threepp::Vector3 carPos, const float carRot,const float speed, const float steer,  const float& dt) const {

        carMesh->position = carPos;
        carMesh->rotation.z = -carRot;

        auto applySteer = [&](const std::shared_ptr<threepp::Group> &wheel, float steerAngle) {
            if (!wheel) return;
            wheel->rotation.z = steerAngle;
        };

        const float wheelRadius = scale_;
        const float rollAmount = speed * dt / wheelRadius;
        auto applyWheelRoll = [&](const std::shared_ptr<threepp::Group> &wheelPivot, float sign = 1.0f) {
            if (!wheelPivot) return;
            if (wheelPivot->children.empty()) return;

            auto wheelMesh = wheelPivot->children.front();
            wheelMesh->rotation.x += rollAmount * sign;
        };

        constexpr float steeringYawFactor = 0.5f;
        applySteer(wheelFrontRMesh,  -steer * steeringYawFactor);
        applySteer(wheelFrontLMesh,  -steer * steeringYawFactor);

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

    static std::shared_ptr<threepp::Group> createPlaceholder(const float scale, const float x, const float y, const float z)
    {
        auto group = threepp::Group::create();
        auto geom = threepp::BoxGeometry::create(x * scale, y * scale, z * scale);
        auto mat  = threepp::MeshStandardMaterial::create();
        mat->color = threepp::Color::rebeccapurple;
        auto mesh = threepp::Mesh::create(geom, mat);
        mesh->position.set(0, 0.5f * scale, 0);
        mesh->material()->needsUpdate();
        group->add(mesh);
        return group;
    }



};

#endif //CARSIM_CARVISUALS_HPP