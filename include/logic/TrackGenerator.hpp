#ifndef CARSIM_TRACKGENERATOR_HPP
#define CARSIM_TRACKGENERATOR_HPP
#include <vector>
#include "entities/Checkpoint.hpp"
#include "entities/Pickup.hpp"
#include "MapSystem.hpp"

struct CheckpointResult {
    std::vector<std::shared_ptr<Checkpoint>> checkpoints;
    size_t startCheckpointIndex;
};

class TrackGenerator {
    public:
        explicit TrackGenerator(MapSystem& map);

        [[nodiscard]] CheckpointResult generateCheckpoints(int startIndex, int numCp) const;

        [[nodiscard]] std::vector<std::shared_ptr<Pickup>> generatePickups() const;

        const std::vector<RoadSample>& getRoadSamples() const;

        const std::vector<RoadSample>& getRoadSamplesReversed() const;

        [[nodiscard]] std::vector<std::pair<threepp::Vector3, float>> generateStartGrid(int amount, int cols, size_t startIdx) const;

    private:
        static size_t sampleAtDistance(
            const std::vector<RoadSample>& samples,
            size_t startIdx,
            float backDistance );

            MapSystem& map_;
            std::vector<RoadSample> roadSamples_;
            std::vector<RoadSample> roadSamplesReversed_;
        };

#endif //CARSIM_TRACKGENERATOR_HPP