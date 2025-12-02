
#include "entities/Checkpoint.hpp"

Checkpoint::Checkpoint(const threepp::Vector3& pos,
           const threepp::Vector3 forward,
           const float width,
           const float depth = 50.f)
    : ICollidable(CollisionType::Checkpoint, pos), forward_(forward), width_(width), depth_(depth) {
    const auto lateral = threepp::Vector2{-forward.y, forward.x};
    rotation_ = std::atan2(lateral.y, lateral.x);
}


[[nodiscard]] OBB2D Checkpoint::getBoundingBox() const{
    return OBB2D::create(
        {position_.x, position_.y},
        {width_ * 0.5f, depth_ * 0.5f},
        threepp::Vector2(forward_.x, forward_.y)
    );
}

void Checkpoint::onCollision(const std::shared_ptr<ICollidable>& other, float dt) {};

CpSnap Checkpoint::snapshot() const {
    CpSnap snap;
    snap.position = position_;
    snap.forward = forward_;
    snap.width = width_;
    snap.start = start_;
    return snap;
}