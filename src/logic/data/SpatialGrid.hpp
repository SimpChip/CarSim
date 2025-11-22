//
// Created by Mella on 10.11.2025.
//

#ifndef CARSIM_SPATIALGRID_HPP
#define CARSIM_SPATIALGRID_HPP

#include "threepp/threepp.hpp"

struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const noexcept {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

template<typename T>
class SpatialGrid {
public:
    float cellSize;
    std::unordered_map<std::pair<int, int>, std::vector<T>, PairHash> grid{};

    explicit SpatialGrid(float cellSize) : cellSize(cellSize) {}

    [[nodiscard]] std::pair<int, int> worldToCell(const threepp::Vector3& pos) const {
        return {static_cast<int>(pos.x / cellSize), static_cast<int>(pos.y / cellSize)};
    }

    void insert(const threepp::Vector3& pos, const T& item) {
        grid[worldToCell(pos)].push_back(item);
    }

    std::vector<T> queryNearby(const threepp::Vector3& pos, int radius = 1) const {
        std::vector<T> result;
        auto [first, second] = worldToCell(pos);
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dy = -radius; dy <= radius; ++dy) {
                auto cell = std::make_pair(first + dx, second + dy);
                auto it = grid.find(cell);
                if (it != grid.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
        return result;
    }
};

#endif //CARSIM_SPATIALGRID_HPP