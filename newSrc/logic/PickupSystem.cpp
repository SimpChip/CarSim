
#include "logic/PickupSystem.hpp"

PickupSystem::PickupSystem() {}


const std::vector<std::shared_ptr<Pickup>>& PickupSystem::getPickups() const { return pickups_; }

void PickupSystem::setPickups(const std::vector<std::shared_ptr<Pickup>>& pickups) {
    pickups_ = pickups;
};

std::vector<PickupSnap> PickupSystem::snapshot() const {
    std::vector<PickupSnap> snap;
    snap.reserve(pickups_.size());
    for (const auto& p : pickups_) {
        snap.push_back(p->snapshot());
    }
    return snap;
}