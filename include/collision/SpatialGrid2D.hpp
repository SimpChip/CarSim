#ifndef CARSIM_SPATIALGRID_HPP
#define CARSIM_SPATIALGRID_HPP

#include <cmath>
#include <algorithm>


// SpatialGrid2D: Spatial hash implementation
// Initial structure generated with AI assistance, adapted for project needs

template<typename T>
class SpatialGrid2D {
public:
    struct Cell {
        int x;
        int y;
        bool operator==(const Cell& o) const noexcept { return x == o.x && y == o.y; }
    };

    struct CellHash {
        size_t operator()(const Cell& k) const noexcept {
            const uint64_t ux = static_cast<uint32_t>(k.x);
            const uint64_t uy = static_cast<uint32_t>(k.y);
            return static_cast<size_t>((ux << 32) ^ uy);
        }
    };

    float cellSize;
    std::unordered_map<Cell, std::vector<T>, CellHash> grid{};

    explicit SpatialGrid2D(const float cellSize) : cellSize(cellSize) {}

    [[nodiscard]] Cell worldToCell(const threepp::Vector2& pos) const {
        const int cx = static_cast<int>(std::floor(pos.x / cellSize));
        const int cy = static_cast<int>(std::floor(pos.y / cellSize));
        return {cx, cy};
    }

    [[nodiscard]] Cell cellFor(const threepp::Vector2& pos) const { return worldToCell(pos); }

    // void insert(const threepp::Vector2& pos, const T& item) {
    //     insertCell(worldToCell(pos), item);
    // }
    //
    // void remove(const threepp::Vector2& pos, const T& item) {
    //     removeCell(worldToCell(pos), item);
    // }

    void insertCell(const Cell& key, const T& item) {
        auto &bucket = grid[key];
        if (!bucket.empty() && bucket.back() == item) return;
        if (std::find(bucket.begin(), bucket.end(), item) == bucket.end()) bucket.push_back(item);
    }

    void removeCell(const Cell& key, const T& item) {
        auto it = grid.find(key);
        if (it == grid.end()) return;
        auto &bucket = it->second;
        bucket.erase(std::remove(bucket.begin(), bucket.end(), item), bucket.end());
        if (bucket.empty()) grid.erase(it);
    }

    [[nodiscard]] threepp::Vector2 cellCenter(const Cell& key, const threepp::Vector2& origin = {0.f, 0.f}) const {
        const float cx = (key.x + 0.5f) * cellSize + origin.x;
        const float cy = (key.y + 0.5f) * cellSize + origin.y;
        return {cx, cy};
    }

    std::vector<T> queryNearby(const threepp::Vector2& pos, const int radius = 1) const {
        std::vector<T> result;
        const Cell c = worldToCell(pos);
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dy = -radius; dy <= radius; ++dy) {
                Cell cell{c.x + dx, c.y + dy};
                auto it = grid.find(cell);
                if (it != grid.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
        return result;
    }
private:
};

#endif //CARSIM_SPATIALGRID_HPP