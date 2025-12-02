#include "logic/LapSystem.hpp"
#include "iostream"
#include "types/events.hpp"

LapSystem::LapSystem(EventBus& bus, const int startIdx) : bus_(bus) , startCheckpointIndex_(startIdx) {
    bus_.subscribe<CollisionEnterEvent>(
        [this](const CollisionEnterEvent& e) {
            if (e.typeA == CollisionType::Car && e.typeB == CollisionType::Checkpoint) {
                handleCheckpointPassed(e.idA, e.idB);
            } else if (e.typeB == CollisionType::Car && e.typeA == CollisionType::Checkpoint) {
                handleCheckpointPassed(e.idB, e.idA);
            }
        }
    );
}

void LapSystem::registerCar(const int carId) {
    if (checkpoints_.empty()) return;
    LapTracker tracker{carId};
    tracker.lapStartTime = raceTimer_;
    tracker.currentCheckpoint = startCheckpointIndex_ % checkpoints_.size();
    tracker.nextCheckpoint = startCheckpointIndex_ % checkpoints_.size();
    trackers_[carId] = tracker;
}

std::vector<std::shared_ptr<Checkpoint>>& LapSystem::getCheckpoints() {
    return checkpoints_;
}

void LapSystem::setCheckpoints(const std::vector<std::shared_ptr<Checkpoint>>& checkpoints) {
    checkpoints_ = checkpoints;
    for (size_t i = 0; i < checkpoints.size(); ++i) {
        cpIdToOrder_[checkpoints[i]->getId()] = static_cast<int>(i);
    }
}


void LapSystem::setStartTimer(const float startTimer) {
    startTimer_ = startTimer;
};
void LapSystem::countDown(const float dt) {
    startTimer_ -= dt;
}
bool LapSystem::hasStarted() const {
    return startTimer_ <= 0;
}

void LapSystem::update(const float dt) {
    raceTimer_ += dt;
}


LapSysSnap LapSystem::snapshot() const {
    LapSysSnap snap;
    snap.startTimer = startTimer_;
    snap.raceTimer = raceTimer_;
    std::vector<CpSnap> checkpointSnaps;
    for (auto& checkpoint : checkpoints_) {
        checkpointSnaps.emplace_back(checkpoint->snapshot());
    }
    snap.startIndex = startCheckpointIndex_;
    snap.checkpoints = checkpointSnaps;
    snap.laptrackers = trackers_;
    return snap;
}

void LapSystem::handleCheckpointPassed(const int carId, const int checkpointId) {
    if (checkpoints_.empty()) {
        return;
    }
    const auto it = cpIdToOrder_.find(checkpointId);
    if (it == cpIdToOrder_.end()) {
        return;
    }
    const int cpIndex = it->second;
    auto& tracker = trackers_[carId];

    if (cpIndex != tracker.nextCheckpoint) return;

    tracker.currentCheckpoint = tracker.nextCheckpoint;

    tracker.nextCheckpoint = (tracker.nextCheckpoint + 1) % static_cast<int>(checkpoints_.size());

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
