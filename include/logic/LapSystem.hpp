
#ifndef CARSIM_CHECKPOINTLOGIC_HPP
#define CARSIM_CHECKPOINTLOGIC_HPP

#include <types/data.hpp>
#include "core/EventBus.hpp"
#include "types/events.hpp"
#include "Checkpoint.hpp"

class LapSystem {
public:
    explicit LapSystem(EventBus& bus, const int startIdx) : bus_(bus) , startCheckpointIndex_(startIdx) {
        bus_.subscribe<CollisionEnterEvent>(

            [this](const CollisionEnterEvent& e) {
                std::cout << "checkpoint passed event" << std::endl;
                if (e.typeA == CollisionType::Car && e.typeB == CollisionType::Checkpoint) {
                    handleCheckpointPassed(e.idA, e.idB);
                } else if (e.typeB == CollisionType::Car && e.typeA == CollisionType::Checkpoint) {
                    handleCheckpointPassed(e.idB, e.idA);
                }
            }
        );
    }

    void registerCar(const int carId) {
        if (checkpoints_.empty()) return;
        LapTracker tracker{carId};
        tracker.lapStartTime = raceTimer_;
        tracker.currentCheckpoint = startCheckpointIndex_ % checkpoints_.size();
        tracker.nextCheckpoint = startCheckpointIndex_ % checkpoints_.size();
        trackers_[carId] = tracker;
    }

    [[nodiscard]] std::vector<std::shared_ptr<Checkpoint>>& getCheckpoints() {
        return checkpoints_;
    }
    void setCheckpoints(const std::vector<std::shared_ptr<Checkpoint>>& checkpoints) {
        checkpoints_ = checkpoints;
        for (size_t i = 0; i < checkpoints.size(); ++i) {
            cpIdToOrder_[checkpoints[i]->getId()] = static_cast<int>(i);
        }
    }

    void setStartTimer(float startTimer);

    bool hasStarted() const {
        return startTimer_ <= 0;
    }
    void countDown(const float dt) {
        startTimer_ -= dt;
    }

    void update(const float dt) {
        raceTimer_ += dt;
    }

    [[nodiscard]] LapManagerData snapshot() const {
        LapManagerData data;
        data.startTimer = startTimer_;
        data.raceTimer = raceTimer_;
        std::vector<CheckpointData> checkpointsData;
        for (auto& checkpoint : checkpoints_) {
            checkpointsData.emplace_back(checkpoint->snapshot());
        }
        data.startIndex = startCheckpointIndex_;
        data.checkpoints = checkpointsData;
        data.laptrackers = trackers_;
        return data;
    }

private:
    EventBus& bus_;

    std::vector<std::shared_ptr<Checkpoint>> checkpoints_;
    std::unordered_map<int, LapTracker> trackers_;
    std::unordered_map<int,int> cpIdToOrder_;
    float startTimer_ = 3;
    int startCheckpointIndex_ = 0;
    float raceTimer_{};


    void handleCheckpointPassed(const int carId, const int checkpointId) {
        std::cout << "handeling" << checkpointId << std::endl;
        if (cpIdToOrder_.empty()) {
            std::cerr << "Checkpoint map is empty!" << std::endl;
            return;
        }
        const auto it = cpIdToOrder_.find(checkpointId);

        if (it == cpIdToOrder_.end()) {
            std::cerr << "Checkpoint " << checkpointId << " not found in map!" << std::endl;
            return;
        }
        std::cout << "it is in order: " << it->first << std::endl;
        const int cpIndex = it->second;
        auto& tracker = trackers_[carId];

        if (cpIndex != tracker.nextCheckpoint) return;
        std::cout << "Passed" << std::endl;
        tracker.currentCheckpoint = tracker.nextCheckpoint;
        std::cout << "Current Passed: " << tracker.currentCheckpoint << "\n";
        tracker.nextCheckpoint = (tracker.nextCheckpoint + 1) % static_cast<int>(checkpoints_.size());
        std::cout << "Next to Pass: " << tracker.nextCheckpoint << "\n";
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