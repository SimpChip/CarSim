//
// Created by Mella on 19.11.2025.
//
#ifndef CARSIM_CHECKPOINTLOGIC_HPP
#define CARSIM_CHECKPOINTLOGIC_HPP

#include <types/data.hpp>
#include "collision/EventBus.hpp"


class Checkpoint : public ICollidable {
public:
    Checkpoint(const bool isStart, const threepp::Vector3& pos,
               const threepp::Vector3& forward,
               const float width,
               const float depth = 50.f)
        : ICollidable(pos, CollisionType::Checkpoint), forward_(forward), width_(width), depth_(depth) {
    }

    [[nodiscard]] OBB2D getBoundingBox() const override {
        threepp::Vector2 fwd(forward_.x, forward_.y);
        const float len = fwd.length();
        if (len < 1e-6f) {
            fwd = threepp::Vector2(0.f, 1.f);
        }
        return OBB2D{
            threepp::Vector2(position_.x, position_.y),
            threepp::Vector2(width_ * 0.5f, depth_ * 0.5f),
            fwd
        };
    }

    void onCollision(const std::shared_ptr<ICollidable>& other) override {}

    CheckpointData snapshot() const {
        CheckpointData data;
        data.position = position_;
        data.forward = forward_;
        data.width = width_;
        data.start = start_;
        return data;
    }
    bool start_{};
    threepp::Vector3 forward_;
    float width_;
    float depth_;
};


class LapManager {
public:
    explicit LapManager(const std::vector<RoadSample>& samples,
                             const float& width,
                             const int startIndex = 0,
                             const float numberOfCp = 20)
        : distanceInterval_(numberOfCp),
          startCheckpointIndex_(startIndex) {
        generateCheckpoints(samples, width, numberOfCp);
        enterHandle_ = EventBus::subscribe<CollisionEnterEvent>(
            [this](const CollisionEnterEvent& e) {
                if (e.typeA == CollisionType::Car && e.typeB == CollisionType::Checkpoint) {
                    handleCheckpointPassed(e.idA, e.idB);
                } else if (e.typeB == CollisionType::Car && e.typeA == CollisionType::Checkpoint) {
                    handleCheckpointPassed(e.idB, e.idA);
                }
            }
        );
    }
    ~LapManager() {
        EventBus::unsubscribe<CollisionEnterEvent>(enterHandle_);
    }

    void registerCar(const int carId) {
        if (checkpoints_.empty()) return;
        LapTracker tracker{carId};
        tracker.lapStartTime = raceTimer_;
        tracker.currentCheckpoint = startCheckpointIndex_ % checkpoints_.size();
        tracker.nextCheckpoint = startCheckpointIndex_ % checkpoints_.size();
        trackers_[carId] = tracker;
    }

    void subscribeEvents() {
        EventBus::subscribe<CollisionEnterEvent>([&](const CollisionEnterEvent& e) {
            if ((e.typeA == CollisionType::Car && e.typeB == CollisionType::Checkpoint)) {
                handleCheckpointPassed(e.idA, e.idB);
            } else if ((e.typeB == CollisionType::Car && e.typeA == CollisionType::Checkpoint)) {
                handleCheckpointPassed(e.idB, e.idA);
            }
        });

        EventBus::subscribe<CollisionExitEvent>([&](const CollisionExitEvent& e) {
            if ((e.typeA == CollisionType::Car && e.typeB == CollisionType::Checkpoint)) {
            } else if ((e.typeB == CollisionType::Car && e.typeA == CollisionType::Checkpoint)) {
            }
        });
    }



    [[nodiscard]] std::vector<std::shared_ptr<Checkpoint>>& getCheckpoints() {
        return checkpoints_;
    }

    void preUpdate() {
        raceTimer_ = 0;
    }
    void update(const float dt) {
        raceTimer_ += dt;
    }

    [[nodiscard]] LapManagerData snapshot() const {
        LapManagerData data;
        data.lapTimer = raceTimer_;
        std::vector<CheckpointData> checkpointsData;
        for (auto& checkpoint : checkpoints_) {
            checkpointsData.emplace_back(checkpoint->snapshot());
        }
        data.checkpoints = checkpointsData;
        data.laptrackers = trackers_;
        return data;
    }

private:
    EventBus::CallbackId enterHandle_{};
    EventBus::CallbackId exitHandle_{};
    std::vector<std::shared_ptr<Checkpoint>> checkpoints_;
    std::unordered_map<int, LapTracker> trackers_;
    std::unordered_map<int,int> cpIdToOrder_;
    float distanceInterval_;
    int startCheckpointIndex_;
    float raceTimer_;

    void generateCheckpoints(const std::vector<RoadSample>& samples, const float width, const int numberOfCp) {

        float totalLength = 0.f;
        for (size_t i = 1; i < samples.size(); ++i) {
            totalLength += (samples[i].pos - samples[i-1].pos).length();
        }
        if (numberOfCp <= 0 || totalLength < 1e-6f) return;

        const float interval = totalLength / numberOfCp;
        float accumulated = 0;
        float nextTarget = interval;
        for (size_t i = 1; i < samples.size(); ++i) {
            const float segLen = (samples[i].pos - samples[i-1].pos).length();
            while ((accumulated + segLen) >= nextTarget) {
                const float targetPercentOfSeg = (nextTarget - accumulated) / segLen;
                auto center = samples[i-1].pos + (samples[i].pos - samples[i-1].pos) * targetPercentOfSeg;
                auto dir = (samples[i].pos - samples[i-1].pos).normalize();
                checkpoints_.emplace_back(std::make_shared<Checkpoint>(false, center, dir, width));
                cpIdToOrder_[checkpoints_.back()->getId()] = checkpoints_.size()-1;
                nextTarget += interval;
            }
            accumulated += segLen;
        }
    }

    void handleCheckpointPassed(const int carId, const int checkpointId) {
        if (cpIdToOrder_.empty()) {
            std::cerr << "Checkpoint map is empty!" << std::endl;
            return;
        }

        const auto it = cpIdToOrder_.find(checkpointId);
        if (it == cpIdToOrder_.end()) {
            std::cerr << "Checkpoint " << checkpointId << " not found in map!" << std::endl;
            return;
        }

        const int checkpointOrder = it->second;
        std::cout << "Checkpoint order = " << checkpointOrder << std::endl;

        const int cpIndex = it->second;
        auto& tracker = trackers_[carId];
        if (cpIndex != tracker.nextCheckpoint) return;

        tracker.nextCheckpoint = (tracker.nextCheckpoint + 1) % checkpoints_.size();

        if (cpIndex == startCheckpointIndex_) {
            if (tracker.currentLap > 0) {
                tracker.lastLapTime = raceTimer_ - tracker.lapStartTime;
                tracker.lapCompleted = true;
                std::cout << "Car " << carId
                          << " completed lap " << tracker.currentLap
                          << " in " << tracker.lastLapTime << " seconds\n";
            }
            tracker.currentLap++;
            tracker.lapStartTime = raceTimer_;
        }
    }

};
#endif //CARSIM_CHECKPOINTLOGIC_HPP