#ifndef CARSIM_PICKUP_HPP
#define CARSIM_PICKUP_HPP
#include "collision/ICollidable.hpp"

class Pickup final : public ICollidable {
public:
    Pickup(const PickupType type, const threepp::Vector3& position, const float respawnTime = 10.0f)
        : ICollidable(CollisionType::SpeedBoost, position), type_(type), respawnTime_(respawnTime) {}

    [[nodiscard]] OBB2D getBoundingBox() const override {
        return OBB2D::create(
            threepp::Vector2(position_.x, position_.y),
            threepp::Vector2(radius_, radius_),
            threepp::Vector2(std::cos(rotation_), std::sin(rotation_))
        );
    }

    [[nodiscard]] PickupType getType() const { return type_; }

    void onCollision(const std::shared_ptr<ICollidable>& other, float dt) override {
        if (active_ && other->getCollisionType() == CollisionType::Car) {
            active_ = false;
            timer_ = respawnTime_;
        }
    }

    void update(const float dt) {
        if (!active_) {
            timer_ -= dt;
            if (timer_ <= 0.0f) {
                active_ = true;
                timer_ = 0.0f;
            }
        } else {
            rotation_ += 0.01 * dt;
            rotation_ = std::fmod(rotation_, 2.0f * static_cast<float>(M_PI));
            if (rotation_ < 0.0f) rotation_ += 2.0f * static_cast<float>(M_PI);
        }
    }

    [[nodiscard]] PickupData snapshot() const {
        PickupData data;
        data.type = type_;
        data.position = position_;
        data.active = active_;
        data.radius = radius_;
        data.respawnTimer = respawnTime_;
        data.rotation = rotation_;
        return data;
    }

private:
    PickupType type_;
    float radius_ = 25.0f;
    float respawnTime_;
    float timer_ = 0.0f;
};

#endif //CARSIM_PICKUP_HPP