//
// Created by Mella on 19.11.2025.
//

#ifndef CARSIM_PICKUPLOGIC_HPP
#define CARSIM_PICKUPLOGIC_HPP


#include <types/data.hpp>

class Pickup final : public ICollidable {
public:
    Pickup(const PickupType type, const threepp::Vector3& position, const float respawnTime = 10.0f)
        : ICollidable(position, CollisionType::SpeedBoost), type_(type), active_(true), respawnTime_(respawnTime) {}

    [[nodiscard]] OBB2D getBoundingBox() const override {
        const float angle = rotation_;
        const threepp::Vector2 forward(std::cos(angle), std::sin(angle));
        return OBB2D{
            threepp::Vector2(position_.x, position_.y),
            threepp::Vector2(radius_, radius_),
            forward,
        };
    }

    [[nodiscard]] PickupType getType() const { return type_; }

    void onCollision(const std::shared_ptr<ICollidable>& other) override {
        std::cout << "PICKUP IS COLLIDED WIHT" << "\n";
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
            rotation_ += dt * 0.1;
            if (rotation_ > 2 * M_PI) rotation_ -= 2 * M_PI;
        }
    }

    [[nodiscard]] bool isActive() const { return active_; }

    PickupData snapshot() const {
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
    bool active_ = true;
    float radius_ = 25.0f;
    float rotation_ = 2.f;
    float respawnTime_;
    float timer_ = 0.0f;
};


class PickupLogic {
public:
    PickupLogic(const std::vector<RoadSample>& samples, const float width) {
        generatePickups(samples, width);
    }

    [[nodiscard]] const std::vector<std::shared_ptr<Pickup>>& getPickups() const { return pickups_; }

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

    void generatePickups(const std::vector<RoadSample>& samples, const float width) {
        pickups_.clear();
        if (samples.size() < 2) return;

        float totalLength = 0.f;
        for (size_t i = 1; i < samples.size(); ++i) {
            totalLength += (samples[i].pos - samples[i-1].pos).length();
        }
        constexpr float pickupSpacing = 200.f;
        float accumulated = 50.f;
        float nextTarget = pickupSpacing;
        bool leftSide = true;

        for (size_t i =  10; i < samples.size()-2; ++i) {
            const auto seg = samples[i].pos - samples[i-1].pos;
            const float segLen = seg.length();
            if (seg.length() < 1e-6f)  {accumulated += segLen; continue;}

            while ((accumulated + segLen) >= nextTarget) {
                const float t = (nextTarget - accumulated) / segLen;
                auto center = samples[i-1].pos + seg * t;
                const auto dir = seg / segLen;
                auto perp = threepp::Vector3(-dir.y, dir.x, 0);
                auto offset = perp * (width * 0.25f);

                auto pickupPos = leftSide ? center + offset : center - offset;
                pickups_.emplace_back(std::make_shared<Pickup>(PickupType::Coin, pickupPos));

                leftSide = !leftSide;
                nextTarget += pickupSpacing;
            }
            accumulated += segLen;
        }

        for (int k = 0; k < 3 && k < (int)getPickups().size(); ++k) {
            auto& p = getPickups()[k];
            auto posGrid = p->getPosition2D();
            auto posOBB  = p->getBoundingBox().center;
            // or however OBB2D exposes center
            std::cout << "Pickup " << p->isActive()
                      << " grid=(" << posGrid.x << "," << posGrid.y << ")"
                      << " obb=("  << posOBB.x  << "," << posOBB.y  << ")\n";
        }
    }
};

#endif //CARSIM_PICKUPLOGIC_HPP