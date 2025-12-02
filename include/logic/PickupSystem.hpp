//
// Created by Mella on 19.11.2025.
//

#ifndef CARSIM_PICKUPLOGIC_HPP
#define CARSIM_PICKUPLOGIC_HPP


#include <types/data.hpp>
#include "../entities/Pickup.hpp"
#include "types/gameSnapshots.hpp"

class PickupSystem {
public:
    PickupSystem();

    [[nodiscard]] const std::vector<std::shared_ptr<Pickup>>& getPickups() const;

    void setPickups(const std::vector<std::shared_ptr<Pickup>>& pickups);

    [[nodiscard]] std::vector<PickupSnap> snapshot() const;
private:
    std::vector<std::shared_ptr<Pickup>> pickups_;
};

#endif //CARSIM_PICKUPLOGIC_HPP