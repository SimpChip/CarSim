#ifndef CARSIM_PICKUP_HPP
#define CARSIM_PICKUP_HPP
#include "collision/ICollidable.hpp"
#include "types/gameSnapshots.hpp"

class Pickup final : public ICollidable {
public:
    Pickup(PickupType type, const threepp::Vector3& position, float respawnTime);

    [[nodiscard]] OBB2D getBoundingBox() const override;
    [[nodiscard]] PickupType getType() const;
    void onCollision(const std::shared_ptr<ICollidable>& other, float dt) override;

    void update(float dt);

    [[nodiscard]] PickupSnap snapshot() const;
private:
    PickupType type_;
    float radius_ = 25.0f;
    float respawnTime_;
    float timer_ = 0.0f;
};

#endif //CARSIM_PICKUP_HPP