
#ifndef CARSIM_CHECKPOINTLOGIC_HPP
#define CARSIM_CHECKPOINTLOGIC_HPP

#include <types/data.hpp>
#include "core/EventBus.hpp"
#include "entities/Checkpoint.hpp"
#include "types/gameSnapshots.hpp"

class LapSystem {
public:
    LapSystem(EventBus& bus, int startIdx);

    void registerCar(int carId);

    [[nodiscard]] std::vector<std::shared_ptr<Checkpoint>>& getCheckpoints();

    void setCheckpoints(const std::vector<std::shared_ptr<Checkpoint>>& checkpoints);

    void setStartTimer(float startTimer);
    bool hasStarted() const;
    void countDown(float dt);

    void update(float dt);

    [[nodiscard]] LapSysSnap snapshot() const;

private:
    EventBus& bus_;
    std::vector<std::shared_ptr<Checkpoint>> checkpoints_;
    std::unordered_map<int, LapTracker> trackers_;
    std::unordered_map<int,int> cpIdToOrder_;
    float startTimer_ = 3;
    int startCheckpointIndex_ = 0;
    float raceTimer_{};

    void handleCheckpointPassed(int carId, int checkpointId);
};
#endif //CARSIM_CHECKPOINTLOGIC_HPP