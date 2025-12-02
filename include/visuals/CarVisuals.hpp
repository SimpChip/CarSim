#ifndef CARSIM_CARVISUALS_HPP
#define CARSIM_CARVISUALS_HPP

#include <iostream>
#include "../helpers/ResourceManager.hpp"
#include "threepp/geometries/BoxGeometry.hpp"
#include "threepp/materials/MeshStandardMaterial.hpp"
#include "threepp/math/MathUtils.hpp"
#include "threepp/math/Vector3.hpp"
#include "threepp/objects/Group.hpp"
#include "threepp/objects/Mesh.hpp"


class CarVisuals {
public:
    CarVisuals(
        const std::string &carName,
        const std::string &color,
        const threepp::Vector3 &carStartPos,
        const float carRot,
        const threepp::Vector3 &carSize,
        const float scale,
        const ResourceManager &resources
    ) : objectName_(carName), scale_(scale) {
        try {
            const auto bodyClone = resources.cloneCarBody(carName, color);
            if (!bodyClone) throw std::runtime_error("Car body clone failed");

            carMesh = threepp::Group::create();
            carMesh->add(bodyClone);

            const auto wheelClones = resources.cloneCarWheels(carName, color);
            const std::vector<threepp::Vector3> wheelAttachPoints = {
                {1.f, 1.05f, 0.5f},
                {-1.f, 1.05f, 0.5f},
                {1.f, -1.05f, 0.5f},
                {-1.f, -1.05f, 0.5f}
            };


            wheels.clear();
            for (size_t i = 0; i < wheelClones.size() && i < wheelAttachPoints.size(); ++i) {
                auto steeringPivot = threepp::Group::create();
                steeringPivot->position = wheelAttachPoints[i];

                if (wheelClones[i]) {
                    auto wheelMesh = std::dynamic_pointer_cast<threepp::Group>(wheelClones[i]->clone(true));
                    steeringPivot->add(wheelMesh);
                } else {
                    steeringPivot->add(createPlaceholder(scale_, 0.5f, 0.5f, 0.5f));
                }

                carMesh->add(steeringPivot);
                wheels.push_back(steeringPivot);

                switch (i) {
                    case 0: wheelFrontLMesh = steeringPivot;
                        break;
                    case 1: wheelFrontRMesh = steeringPivot;
                        break;
                    case 2: wheelBackLMesh = steeringPivot;
                        break;
                    case 3: wheelBackRMesh = steeringPivot;
                        break;
                    default: throw std::runtime_error("More than 4 wheels should not exist");
                }
            }

            threepp::Box3 bounds;
            bounds.setFromObject(*carMesh);

            threepp::Vector3 size = bounds.getSize();

            float scaleX = carSize.x / size.x;
            float scaleY = carSize.y / size.y;
            float scaleZ = carSize.z / size.z;


            carMesh->scale = threepp::Vector3(scaleX, scaleY, scaleZ);
        } catch (const std::exception &ex) {
            std::cerr << "Failed to clone car model: " << ex.what() << "\n";
            carMesh = createPlaceholder(scale_, 2.f, 4.f, 1.f);
            carMesh->rotation.x = -threepp::math::PI / 2.f;
        }
        carMesh->position = carStartPos;
        carMesh->rotation.z = -carRot;
        carMesh->name = objectName_;

        carMesh->position = carStartPos;
        carMesh->name = carName;
    }

    std::shared_ptr<threepp::Group> getCarMesh() {
        return carMesh;
    }

    void update(
        const threepp::Vector3 &carPos,
        const float carRot,
        const float speed,
        const float steer,
        const float dt
    ) const {
        carMesh->position = carPos;
        carMesh->rotation.z = -carRot;

        constexpr float steeringYawFactor = 0.5f;

        const float wheelRadius = scale_;
        const float rollAmount = speed * dt / wheelRadius;

        auto applyWheel = [&](const std::shared_ptr<threepp::Group> &pivot, float steerAngle, float rollSign) {
            if (!pivot) return;

            pivot->rotation.z = steerAngle;

            if (!pivot->children.empty()) {
                auto wheelMesh = pivot->children.front();
                wheelMesh->rotation.x += rollAmount * rollSign;
            }
        };

        applyWheel(wheelFrontLMesh, -steer * steeringYawFactor, +1.0f);
        applyWheel(wheelFrontRMesh, -steer * steeringYawFactor, +1.0f);

        applyWheel(wheelBackLMesh, 0.0f, +1.0f);
        applyWheel(wheelBackRMesh, 0.0f, +1.0f);
    }

private:
    std::shared_ptr<threepp::Group> carMesh;
    std::string objectName_;
    std::shared_ptr<threepp::Group> carBodyMesh;
    std::shared_ptr<threepp::Group> wheelFrontLMesh, wheelFrontRMesh, wheelBackLMesh, wheelBackRMesh;
    std::vector<std::shared_ptr<threepp::Group> > wheels;
    float scale_;

    static std::shared_ptr<threepp::Group> createPlaceholder(const float scale, const float x, const float y,
                                                             const float z) {
        auto group = threepp::Group::create();
        auto geom = threepp::BoxGeometry::create(x * scale, y * scale, z * scale);
        auto mat = threepp::MeshStandardMaterial::create();
        mat->color = threepp::Color::rebeccapurple;
        auto mesh = threepp::Mesh::create(geom, mat);
        mesh->position.set(0, 0.5f * scale, 0);
        mesh->material()->needsUpdate();
        group->add(mesh);
        return group;
    }
};

#endif //CARSIM_CARVISUALS_HPP
