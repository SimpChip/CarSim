#include "entities/Pickup.hpp"

Pickup::Pickup(
    const PickupType type,
    const threepp::Vector3& position,
    const float respawnTime = 10.0f)
:   ICollidable(CollisionType::SpeedBoost, position),
    type_(type),
    respawnTime_(respawnTime)
{}

[[nodiscard]] OBB2D Pickup::getBoundingBox() const {
    return OBB2D::create(
        threepp::Vector2(position_.x, position_.y),
        threepp::Vector2(radius_, radius_),
        threepp::Vector2(std::cos(rotation_), std::sin(rotation_))
    );
}
void Pickup::onCollision(const std::shared_ptr<ICollidable>& other, float dt) {
    if (active_ && other->getCollisionType() == CollisionType::Car) {
        active_ = false;
        timer_ = respawnTime_;
    }
}

[[nodiscard]] PickupType Pickup::getType() const { return type_; }

void Pickup::update(const float dt) {
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

PickupSnap Pickup::snapshot() const {
    PickupSnap snap;
    snap.type = type_;
    snap.position = position_;
    snap.active = active_;
    snap.radius = radius_;
    snap.respawnTimer = respawnTime_;
    snap.rotation = rotation_;
    return snap;
}