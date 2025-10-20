//
// Created by Mella on 18.10.2025.
//

#ifndef CARSIM_RACEGAME_HPP
#define CARSIM_RACEGAME_HPP
#include <map>
#include <threepp/threepp.hpp>
#include <cmath>
#include <CarLogic.hpp>

using Vector3 = threepp::Vector3;

struct Checkpoint {
    std::array<Vector3, 2> points;
    int count = 0; // how many points added

    void add(const Vector3& p) {
        if (count < 2) {
            points[count++] = p;
        }
    }
    [[nodiscard]] Vector3 midpoint() const {
        if (count < 2) return {0,0,0};
        return (points[0] + points[1]) / 2.0f;
    }
    [[nodiscard]] float angle() const {
        const float angle = std::atan2(points[1].z - points[0].z, points[1].x - points[0].x);
        return angle;
    }
};

class RaceGame
{
    public:
        std::vector<uint8_t> roadMask;      // 1 bit per pixel
        std::vector<uint8_t> obstacleMask;

        std::map<int, Checkpoint > checkpointMap;
        explicit RaceGame(const std::string& filename);
        void handleMapCollisions(CarLogic& car);
        void setBit(std::vector<uint8_t>& mask, int x, int y);
        int getBit(const std::vector<uint8_t>& mask, int x, int y);
        std::pair<Vector3, float> getStartPositionAndRotation() const;

    private:
        int width;
        int height;

};

#endif //CARSIM_RACEGAME_HPP