
#ifndef CARSIM_CHECKPOINT_HPP
#define CARSIM_CHECKPOINT_HPP

#include "collision/ICollidable.hpp"
class Checkpoint : public ICollidable {
public:
    Checkpoint(const threepp::Vector3& pos,
               const threepp::Vector3 forward,
               const float width,
               const float depth = 50.f)
        : ICollidable(CollisionType::Checkpoint, pos), forward_(forward), width_(width), depth_(depth) {
        const auto lateral = threepp::Vector2{-forward.y, forward.x};
        rotation_ = std::atan2(lateral.y, lateral.x);
    }

    [[nodiscard]] OBB2D getBoundingBox() const override {
        return OBB2D::create(
            {position_.x, position_.y},
            {width_ * 0.5f, depth_ * 0.5f},
            threepp::Vector2(forward_.x, forward_.y)
        );
    }

    void onCollision(const std::shared_ptr<ICollidable>& other, float dt) override {}

    [[nodiscard]] CheckpointData snapshot() const {
        CheckpointData data;
        data.position = position_;
        data.forward = forward_;
        data.width = width_;
        data.start = start_;
        data.corners = getBoundingBox().getCorners();
        return data;
    }
    bool start_{};
    threepp::Vector3 forward_;
    float width_;
    float depth_;
};

#endif //CARSIM_CHECKPOINT_HPP