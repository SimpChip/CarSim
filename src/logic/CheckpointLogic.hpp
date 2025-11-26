//
// Created by Mella on 19.11.2025.
//

#ifndef CARSIM_CHECKPOINTLOGIC_HPP
#define CARSIM_CHECKPOINTLOGIC_HPP
#include <threepp/threepp.hpp>
#include <types/data.hpp>
#include <cmath>
#include <algorithm>

class CheckpointLogic {
    public:
        explicit CheckpointLogic(const std::vector<RoadSample>& samples, const float width, const int start, const float distanceInterval = 3000) :
            distanceInterval_(distanceInterval),
            startCheckpointIndex_(start),
            currentCheckpointIndex_(start)

            {
                generateCheckpoints(samples, width);
            }

        [[nodiscard]] std::pair<threepp::Vector3, float> getStartPositionAndRotation(const int i, const int count) const {

                const threepp::Vector3 forward = startCheckpoint_.forward;
                const threepp::Vector3 lateral(-forward.y, forward.x, 0);

                const float rot = std::atan2(forward.x, forward.y);

                constexpr float backSpacing = 100.0f;
                constexpr float sideSpacing = 50.0f;
                const int gridCol = i % 2;
                const int gridRow = i / 2;

                threepp::Vector3 pos = startCheckpoint_.position
                            - forward * (static_cast<float>(gridRow) * backSpacing)
                            + lateral * (static_cast<float>(gridCol) * sideSpacing);

                return {pos, rot};
        }

        void handleCheckpointPassing(const threepp::Vector3& currPos, const threepp::Vector3& prevPos) {
            if (checkpoints_.empty()) return;

            if (currentCheckpointIndex_ < checkpoints_.size()) {
                Checkpoint& cp = checkpoints_[currentCheckpointIndex_];
                if (hasCarPassedCheckpoint(currPos, prevPos, cp)) {
                    cp.passed = true;
                    currentCheckpointIndex_++;
                }
            }
            else {
                if (hasCarPassedCheckpoint(currPos, prevPos, startCheckpoint_) && allCheckpointsPassed()) {
                    resetCheckpoints();
                    lapCompleted_ = true;
                }
            }
            lapCompleted_ = false;
        }

        [[nodiscard]] bool allCheckpointsPassed() const {
            return std::ranges::all_of(checkpoints_,
                [](const auto& cp) { return cp.passed; });
        }

        [[nodiscard]] bool lapCompleted() const { return lapCompleted_; }

        void resetCheckpoints() {
            for (auto& cp : checkpoints_) cp.passed = false;
            currentCheckpointIndex_ = 0;
        }

        CheckpointData snapshot() const {
            CheckpointData data;
            data.checkpoints = checkpoints_;
            data.startpoint = startCheckpoint_;
            data.allCheckPassed = lapCompleted_;
            data.currentCheckpointIndex = currentCheckpointIndex_;
            data.startCheckpointIndex = startCheckpointIndex_;

            return data;
        }
        [[nodiscard]] int getRoadSampleStartIndex() const {return roadSampleStartIndex_;}

    private:
        std::vector<Checkpoint> checkpoints_;
        Checkpoint startCheckpoint_;
        float distanceInterval_;
        int roadSampleStartIndex_{};
        int startCheckpointIndex_;
        int currentCheckpointIndex_;
        bool lapCompleted_ = false;

        void generateCheckpoints(const std::vector<RoadSample>& samples, const float width) {
            int checkCount = 0;
            float accumulated = 0.0f;
            for (size_t i = 1; i < samples.size(); ++i) {
                const threepp::Vector3 p0 = samples[i-1].pos;
                const threepp::Vector3 p1 = samples[i].pos;
                const float segLength = (p1 - p0).length();
                accumulated += segLength;
                if (accumulated >= distanceInterval_) {
                    accumulated = 0.0f;

                    const threepp::Vector3 dir = (p1 - p0).normalize();
                    const threepp::Vector3 center = (p0 + p1) * 0.5f; // midpoint

                    Checkpoint cp;
                    cp.id = checkCount++;
                    cp.position = center;
                    cp.forward = dir;
                    cp.width = width;
                    cp.passed = false;

                    if (checkCount-1 != startCheckpointIndex_) {
                        checkpoints_.push_back(cp);
                    } else {
                        startCheckpoint_ = cp;
                        roadSampleStartIndex_ = static_cast<int>(i);
                    }
                }
            }
            currentCheckpointIndex_ = startCheckpointIndex_;
        };


        static bool hasCarPassedCheckpoint(const threepp::Vector3& currPos, const threepp::Vector3& prevPos, const Checkpoint& cp) {
            const threepp::Vector3 normal = cp.forward;
            const threepp::Vector3 toPrev = prevPos - cp.position;
            const threepp::Vector3 toCurr = currPos - cp.position;

            const float dPrev = normal.dot(toPrev);
            const float dCurr = normal.dot(toCurr);
            const bool passed = (dPrev <= 0 && dCurr > 0) || (dPrev >= 0 && dCurr < 0);
            if (!passed) return false;

            const threepp::Vector3 lateral = threepp::Vector3(-normal.y, normal.x, 0).normalize();
            const float offset = (currPos - cp.position).dot(lateral);

            return std::abs(offset) <= cp.width * 0.5f;
        }


};
#endif //CARSIM_CHECKPOINTLOGIC_HPP