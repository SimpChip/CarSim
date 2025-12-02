#include "logic/TrackGenerator.hpp"
#include "threepp/input/KeyListener.hpp"

TrackGenerator::TrackGenerator(MapSystem& map) : map_(map) {
    roadSamples_ = map_.road.getSamples();
    roadSamplesReversed_ = roadSamples_;
    std::reverse(roadSamplesReversed_.begin(), roadSamplesReversed_.end());
}

CheckpointResult TrackGenerator::generateCheckpoints(const int startIndex,const int numCp) const {
    const auto samples = map_.road.getSamples();
    const auto roadWidth = map_.road.getWidth() * 1.1f;
    std::vector<std::shared_ptr<Checkpoint>> checkpoints;
    size_t startCheckpointIndex = 0;

    float totalLength = 0.f;
    for (size_t i = 1; i < samples.size(); ++i) {
        totalLength += (samples[i].pos - samples[i-1].pos).length();
    }
    if (numCp <= 0 || totalLength < 1e-6f) return CheckpointResult{};

    const float interval = totalLength / numCp;
    float accumulated = 0;
    float nextTarget = interval;
    int counter = 0;
    for (size_t i = 1; i < samples.size(); ++i) {
        const float segLen = (samples[i].pos - samples[i-1].pos).length();
        while ((accumulated + segLen) >= nextTarget) {
            const float targetPercentOfSeg = (nextTarget - accumulated) / segLen;
            auto center = samples[i-1].pos + (samples[i].pos - samples[i-1].pos) * targetPercentOfSeg;
            const auto dir = (samples[i].pos - samples[i-1].pos).normalize();
            checkpoints.emplace_back(std::make_shared<Checkpoint>(center, dir, roadWidth, 50));
            if (startIndex == counter) {
                startCheckpointIndex = i;
            }
            nextTarget += interval;

            counter++;
        }
        accumulated += segLen;
    }
    return CheckpointResult{checkpoints, startCheckpointIndex};
};

//TODO Make helper function for both genrate pickup and checkpoints, using the same approch for generation

std::vector<std::shared_ptr<Pickup>> TrackGenerator::generatePickups() const {
    const auto samples = map_.road.getSamples();
    const auto roadWidth = map_.road.getWidth();
    std::vector<std::shared_ptr<Pickup>> pickups;

    if (samples.size() < 2) return {};

    float totalLength = 0.f;
    for (size_t i = 1; i < samples.size(); ++i) {
        totalLength += (samples[i].pos - samples[i-1].pos).length();
    }

    if (totalLength < 1e-6f) return std::vector<std::shared_ptr<Pickup>>();

    constexpr float pickupSpacing = 1000.f;
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
            auto offset = perp * (roadWidth * 0.25f);

            auto pickupPos = leftSide ? center + offset : center - offset;
            pickups.emplace_back(std::make_shared<Pickup>(PickupType::Coin, pickupPos, 10));

            leftSide = !leftSide;
            nextTarget += pickupSpacing;
        }
        accumulated += segLen;
    }
    return pickups;
}

std::vector<std::pair<threepp::Vector3, float>> TrackGenerator::generateStartGrid(
    const int amount,
    const int cols,
    const size_t startIdx
) const {
    const auto samples = map_.road.getSamples();
    const auto roadWidth = map_.road.getWidth();
    std::vector<std::pair<threepp::Vector3, float>> positions;

    if (samples.size() < 2 || amount <= 0 || cols <= 0 || startIdx >= samples.size() - 1) {
        std::cout << "invalid input in generate start grid\n";
        return positions;
    }

    const int numRows = (amount + cols - 1) / cols;
    int carIndex = 0;

    for (int row = 0; row < numRows && carIndex < amount; ++row) {
        constexpr float startOffset = 80.f;
        constexpr float rowSpacing  = 60.f;
        const float backDistance = startOffset + row * rowSpacing;


        const size_t idx = sampleAtDistance(samples, startIdx, backDistance);
        const auto& rowSample = samples[idx];
        const auto& nextSample = samples[(idx + 1) % samples.size()];


        const threepp::Vector3 forwardDir = (nextSample.pos - rowSample.pos).normalize();
        float rotation = std::atan2(forwardDir.x, forwardDir.y);

        threepp::Vector3 rightDir(-forwardDir.y, forwardDir.x, 0);

        const int carsInThisRow = std::min(cols, amount - carIndex);
        for (int col = 0; col < carsInThisRow; ++col) {

            constexpr float carSpacingBetween = 30.f;
            float lateralOffset = (col - (carsInThisRow - 1) * 0.5f) * carSpacingBetween;
            float maxOffset = roadWidth * 0.8f;
            const float clampedOffset = std::clamp(lateralOffset, -maxOffset, maxOffset);

            threepp::Vector3 carPos = rowSample.pos + rightDir * clampedOffset;
            positions.emplace_back(carPos, rotation);
            carIndex++;
        }
    }

    return positions;
}

size_t TrackGenerator::sampleAtDistance(
    const std::vector<RoadSample>& samples,
    const size_t startIdx,
    const float backDistance
) {
    float remaining = backDistance;
    size_t idx = startIdx;
    while (remaining > 0 && idx > 0) {
        const auto& curr = samples[idx];
        const auto& prev = samples[idx - 1];
        const float segLen = (curr.pos - prev.pos).length();
        if (segLen >= remaining) {
            return idx;
        }
        remaining -= segLen;
        idx--;
    }
    return 0;
}


const std::vector<RoadSample>& TrackGenerator::getRoadSamples() const {
    return roadSamples_;
}

const std::vector<RoadSample>& TrackGenerator::getRoadSamplesReversed() const {
    return roadSamplesReversed_;
}

