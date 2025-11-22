//
// Created by Mella on 19.11.2025.
//

#ifndef CARSIM_PICKUPLOGIC_HPP
#define CARSIM_PICKUPLOGIC_HPP

#include <threepp/threepp.hpp>
#include <data/types.h>
#include <data/SpatialGrid.hpp>
#include <CarLogic.hpp>
class PickupLogic {
    public:
        PickupLogic(const std::vector<RoadSample>& samples, const float width): pickupGrid_(100) {
            generatePickups(samples, width);
        }

        void handlePickupCollisions(CarLogic& car, const float dt) {
            const OBB carBox = car.getOrientedBoundingBox();

            for (const auto nearbyIndices = pickupGrid_.queryNearby(car.getPosition(), 1); size_t idx : nearbyIndices) {
                Pickup& pickup = pickups_[idx];
                if (!pickup.active) continue;

                if (isPickupCollidingWithOBB(pickup.position, carBox, pickup.radius)) {
                    pickup.active = false;
                    pickup.respawnTimer = 5.0f;
                    inactivePickupIndices_.push_back(idx);
                    const float bfactor = car.getBoostFactor();
                    car.setBoostFactor(bfactor + 6 * std::pow(0.4f,bfactor ));
                }
            }
        }

        std::vector<Pickup> getPickups() const {return pickups_;}

        void update(const float dt) {
            auto it = inactivePickupIndices_.begin();
            while (it != inactivePickupIndices_.end()) {
                Pickup& pickup = pickups_[*it];
                pickup.respawnTimer -= dt;
                if (pickup.respawnTimer <= 0.0f) {
                    pickup.active = true;
                    it = inactivePickupIndices_.erase(it);
                } else {
                    ++it;
                }
            }

        }
    private:
        std::vector<Pickup> pickups_;
        SpatialGrid<size_t> pickupGrid_;
        std::vector<size_t> inactivePickupIndices_;

        static bool isPickupCollidingWithOBB(const Vector3& pickupPos, const OBB& box, const float pickupRadius) {
            const Vector3 toPickup = pickupPos - box.center;
            const float xProj = toPickup.dot(box.right);
            const float yProj = toPickup.dot(box.forward);

            return std::abs(xProj) <= box.halfExtents.x + pickupRadius &&
                   std::abs(yProj) <= box.halfExtents.y + pickupRadius;
        }

        void generatePickups(const std::vector<RoadSample>& samples, const float width)  {

            const size_t sampSize = samples.size();
            constexpr int pickupInterval = 30;
            bool leftSide = true;

            for (size_t i = pickupInterval; i < sampSize; i += pickupInterval) {
                const threepp::Vector3 p0 = samples[i - 1].pos;
                const threepp::Vector3 p1 = samples[i].pos;
                const threepp::Vector3 dir = (p1 - p0).normalize();
                const threepp::Vector3 perp(-dir.y, dir.x, 0);

                const threepp::Vector3 center = samples[i].pos;
                const threepp::Vector3 offset = perp * (width * 0.25f);

                const threepp::Vector3 pickupPos1 = leftSide ? center + offset : center - offset;
                pickups_.emplace_back(Pickup{PickupType::Coin, pickupPos1});
                leftSide = !leftSide;
            }

            for (size_t i = 0 ; i < pickups_.size(); i ++) {
                pickupGrid_.insert(pickups_[i].position, i);
            }
        }
};

#endif //CARSIM_PICKUPLOGIC_HPP