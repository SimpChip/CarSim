
#ifndef CARSIM_CHECKPOINT_HPP
#define CARSIM_CHECKPOINT_HPP

#include "collision/ICollidable.hpp"
#include "types/gameSnapshots.hpp"

class Checkpoint final : public ICollidable {
public:
    Checkpoint(const threepp::Vector3& pos,
               const threepp::Vector3 forward,
               float width,
               float depth);

    [[nodiscard]] OBB2D getBoundingBox() const override;

    void onCollision(const std::shared_ptr<ICollidable>& other, float dt) override;

    [[nodiscard]] CpSnap snapshot() const;
private:
    bool start_{};
    threepp::Vector3 forward_;
    float width_;
    float depth_;
};

#endif //CARSIM_CHECKPOINT_HPP