//
// Created by Mella on 29.10.2025.
//

#ifndef CARSIM_PICKUPVISUALS_HPP
#define CARSIM_PICKUPVISUALS_HPP

#include <threepp/threepp.hpp>
#include <MapLogic.hpp>

class PickupVisuals {
public:
    MapLogic& mapLogic_;
    std::vector<std::shared_ptr<threepp::Group>> pickupMeshes_;

    explicit PickupVisuals(MapLogic& mapLogic): mapLogic_(mapLogic)  {

        threepp::OBJLoader ol;

        for (auto&pickup : mapLogic_.Pickups_) {
            std::shared_ptr<threepp::Group> mesh;
            switch (pickup.type) {
                case PickupType::Coin: {
                    std::string Dir = std::string(DATA_DIR) + "/objects/Pickups/coin.obj";
                    mesh = ol.load(Dir, true);
                    break;
                }
            }

            if (mesh) {
                mesh->position.set(pickup.position.x * mapLogic_.scale_, pickup.position.y * mapLogic_.scale_, 0);
                mesh->scale = Vector3(pickup.radius * 2.f,pickup.radius * 2.f, pickup.radius * 2.f);
                pickupMeshes_.push_back(mesh);
            }
        }


    }

    void addToScene(const std::shared_ptr<threepp::Scene>& scene) const {
        for (auto& mesh : pickupMeshes_) {
            scene->add(mesh);
        }
    }

    void update(const float dt) {
        for (size_t i = 0; i < mapLogic_.Pickups_.size(); ++i) {
            auto& pickup = mapLogic_.Pickups_[i];
            if (i < pickupMeshes_.size()) {
                pickupMeshes_[i]->visible = pickup.active;
                pickupMeshes_[i]->rotation.z += dt * 8.f;
                pickupMeshes_[i]->position = pickup.position;
            }
        }
    }


private:


};

#endif //CARSIM_PICKUPVISUALS_HPP