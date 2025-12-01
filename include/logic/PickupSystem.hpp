//
// Created by Mella on 19.11.2025.
//

#ifndef CARSIM_PICKUPLOGIC_HPP
#define CARSIM_PICKUPLOGIC_HPP


#include <types/data.hpp>
#include "Pickup.hpp"

class PickupSystem {
public:
    PickupSystem() {};

    [[nodiscard]] const std::vector<std::shared_ptr<Pickup>>& getPickups() const { return pickups_; }

    void setPickups(const std::vector<std::shared_ptr<Pickup>>& pickups) {
        pickups_ = pickups;
    };

    [[nodiscard]] std::vector<PickupData> snapshot() const {
        std::vector<PickupData> data;
        data.reserve(pickups_.size());
        for (const auto& p : pickups_) {
            data.push_back(p->snapshot());
        }
        return data;
    }

private:
    std::vector<std::shared_ptr<Pickup>> pickups_;
};

#endif //CARSIM_PICKUPLOGIC_HPP