//
// Created by Mella on 29.10.2025.
//

#ifndef CARSIM_PICKUPVISUALS_HPP
#define CARSIM_PICKUPVISUALS_HPP

#include <threepp/threepp.hpp>
#include <PickupLogic.hpp>

class PickupVisuals {
public:
    std::vector<std::shared_ptr<threepp::Group>> pickupMeshes_;

    explicit PickupVisuals(const std::vector<Pickup>& pickups, const float scale)  {
        threepp::OBJLoader ol;
        for (auto&pickup : pickups) {
            std::shared_ptr<threepp::Group> mesh;
            switch (pickup.type) {
                case PickupType::Coin: {
                    std::string Dir = std::string(DATA_DIR) + "/objects/Pickups/coin.obj";
                    mesh = ol.load(Dir, true);
                    break;
                }
            }

            if (mesh) {
                mesh->position.set(pickup.position.x * scale, pickup.position.y * scale, 0);
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

    void update(const std::vector<Pickup>& pickups, const float dt) const {
        for (size_t i = 0; i < pickups.size(); ++i) {
            auto& pickup = pickups[i];
            if (i < pickupMeshes_.size()) {
                pickupMeshes_[i]->visible = pickup.active;
                pickupMeshes_[i]->rotation.z += dt * 8.f;
                pickupMeshes_[i]->position = pickup.position;
            }
        }
    }
};

#endif //CARSIM_PICKUPVISUALS_HPP