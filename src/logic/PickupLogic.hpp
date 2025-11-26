//
// Created by Mella on 19.11.2025.
//

#ifndef CARSIM_PICKUPLOGIC_HPP
#define CARSIM_PICKUPLOGIC_HPP

#include <threepp/threepp.hpp>
#include <types/data.hpp>
#include <data/SpatialGrid.hpp>
#include <CarLogic.hpp>

class Pickup : public ICollidable {
public:
    Pickup(const PickupType type, const threepp::Vector3& position, const float respawnTime = 10.0f)
        : type_(type), position_(position), respawnTime_(respawnTime) {}

    [[nodiscard]] threepp::Vector3 getPosition() const override { return position_; }
    [[nodiscard]] OBB getBoundingBox() const override {
        return OBB(position_, threepp::Vector3(radius_, radius_, radius_));
    }
    [[nodiscard]] CollisionType getCollisonType() const override {
        switch (type_) {
            case PickupType::Coin: return CollisionType::SpeedBoost;
            case PickupType::BadCoin: return CollisionType::SlowBoost;
            default: return CollisionType::SpeedBoost;
        }
    }
    [[nodiscard]] PickupType getType() const { return type_; }

    void onCollision(ICollidable* other) override {
        if (active_ && other->getCollisonType() == CollisionType::Car) {
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
        }
    }

    [[nodiscard]] bool isActive() const { return active_; }

private:
    PickupType type_;
    threepp::Vector3 position_;
    bool active_ = true;
    float radius_ = 6.0f;
    float respawnTime_;
    float timer_ = 0.0f;
};


class PickupLogic {
public:
    PickupLogic(const std::vector<RoadSample>& samples, const float width) {
        generatePickups(samples, width);
    }

    [[nodiscard]] const std::vector<Pickup>& getPickups() const { return pickups_; }
    [[nodiscard]] std::vector<Pickup>& getPickups() { return pickups_; }

    [[nodiscard]] std::vector<PickupData> snapshot() const {
        std::vector<PickupData> data;
        data.reserve(pickups_.size());
        for (const auto& p : pickups_) {
            data.push_back(PickupData{p.getType(), p.getPosition(), p.isActive()});
        }
        return data;
    }

private:
    std::vector<Pickup> pickups_;

    void generatePickups(const std::vector<RoadSample>& samples, float width) {
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

            const threepp::Vector3 pickupPos = leftSide ? center + offset : center - offset;
            pickups_.emplace_back(PickupType::Coin, pickupPos);
            leftSide = !leftSide;
        }
    }
};

#endif //CARSIM_PICKUPLOGIC_HPP