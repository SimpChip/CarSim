//
// Created by Mella on 18.10.2025.
//
#ifndef MAPLOGIC_HPP
#define MAPLOGIC_HPP

#include <fstream>
#include "threepp/threepp.hpp"
#include "CarLogic.hpp"
#include "RoadLogic.hpp"
#include <data/SpatialGrid.hpp>
#include <utility>
#include "types/data.hpp"


enum class BorderType {
    Canyon,
    RollingHills,
    Forest
};

struct WallSegment {
    Vector3 p1;
    Vector3 p2;
    Vector3 normal;
};

class MapLogic {
public:
    RoadLogic road;

    short width_{};
    short height_{};
    short scale_ = 1;
    std::string filename_;

    SpatialGrid<WallSegment> wallGrid;
    std::vector<Vector3> borderPoints;
    BorderType currentBiome = BorderType::Forest;

    explicit MapLogic(std::string filename) :
        road(250, true),
        filename_(std::move(filename)),
        wallGrid(200.0f)
    {
        if (filename_ == "firstMap") {
            width_ = 10000;
            height_ = 10000;
            road.addPoint({1000, 1000, 0});
            road.addPoint({3000, 1500, 0});
            road.addPoint({5000, 1200, 0});
            road.addPoint({7000, 2000, 0});
            road.addPoint({8500, 3500, 0});
            road.addPoint({9000, 5000, 0});
            road.addPoint({8500, 6500, 0});
            road.addPoint({7000, 8000, 0});
            road.addPoint({5000, 8800, 0});
            road.addPoint({3000, 8000, 0});
            road.addPoint({1500, 6500, 0});
            road.addPoint({1200, 4000, 0});
            road.buildSmoothSamples();
            road.setLooped(true);
        }
        else if (filename_ == "secondMap") {
            width_ = 15000;
            height_ = 15000;
            road.addPoint({500,  3000, 0});
            road.addPoint({4500,  3700, 0});
            road.addPoint({7000,  3300, 0});
            road.addPoint({9500, 4000, 0});
            road.addPoint({12300, 5500, 0});
            road.addPoint({13500, 7700, 0});
            road.addPoint({13000, 9500, 0});
            road.addPoint({11500,  11700, 0});
            road.addPoint({8500, 12600,0});
            road.addPoint({5500,  12100 , 0});
            road.addPoint({3300,   10100, 0});
            road.addPoint({2100,  6700, 0});
            road.buildSmoothSamples();
            road.setLooped(true);
        }
        else if (filename_ == "thirdMap") {
            width_ = 5000;
            height_ = 5000;
            road.addPoint({1200, 1200, 0});
            road.addPoint({2000, 1700, 0});
            road.addPoint({2600, 1400, 0});
            road.addPoint({3300, 1800, 0});
            road.addPoint({4000, 1500, 0});
            road.addPoint({4600, 2000, 0});
            road.addPoint({4100, 2600, 0});
            road.addPoint({3500, 3100, 0});
            road.addPoint({2700, 2900, 0});
            road.addPoint({2000, 3400, 0});
            road.addPoint({1500, 3000, 0});
            road.addPoint({1300, 2200, 0});
            road.addPoint({1000, 1700, 0});
            road.buildSmoothSamples();
            road.setLooped(true);
        }
        else if (filename_ == "fourthMap") {
            width_ = 10000;
            height_ = 10000;

            road.addPoint({5000, 3500, 0});
            road.addPoint({6500, 3500, 0});
            road.addPoint({8000, 3500, 0});
            road.addPoint({9000, 2500, 0});
            road.addPoint({8000, 1500, 0});
            road.addPoint({7000, 2000, 0});
            road.addPoint({6500, 3000, 0});
            road.addPoint({6500, 3500, 0});
            road.addPoint({6500, 5000, 0});
            road.addPoint({6500, 6500, 0});
            road.addPoint({6500, 8000, 0});
            road.addPoint({7500, 9000, 0});
            road.addPoint({8500, 8000, 0});
            road.addPoint({8000, 7000, 0});
            road.addPoint({7000, 6500, 0});
            road.addPoint({6500, 6500, 0});
            road.addPoint({5000, 6500, 0});
            road.addPoint({3500, 6500, 0});
            road.addPoint({2000, 6500, 0});
            road.addPoint({1000, 7500, 0});
            road.addPoint({2000, 8500, 0});
            road.addPoint({3000, 8000, 0});
            road.addPoint({3500, 7000, 0});
            road.addPoint({3500, 6500, 0});
            road.addPoint({3500, 5000, 0});
            road.addPoint({3500, 3500, 0});
            road.addPoint({3500, 2000, 0});
            road.addPoint({2500, 1000, 0});
            road.addPoint({1500, 2000, 0});
            road.addPoint({2000, 3000, 0});
            road.addPoint({3000, 3500, 0});
            road.addPoint({3500, 3500, 0});
            road.addPoint({4000, 3500, 0});
            road.buildSmoothSamples();
            road.setLooped(true);
        }
        generateOrganicBorder();
    }

    [[nodiscard]] bool isLooped() const {
        return road.isLooped() ;
    }

    [[nodiscard]] int getWidth() const {return width_;}
    [[nodiscard]] int getHeight() const {return height_;}

    [[nodiscard]] bool isOutsideRoad(const Vector3& pos) const {
        const auto roadSamples = road.getSamples();
        for (size_t i = 0; i < roadSamples.size() - 1; ++i) {
            Vector3 p0 = roadSamples[i].pos;
            Vector3 p1 = roadSamples[i + 1].pos;
            Vector3 seg = p1 - p0;
            Vector3 toPos = pos - p0;

            const float segLenSq = seg.dot(seg);
            if (segLenSq < 1e-6f) continue;

            const float t = std::clamp(toPos.dot(seg) / segLenSq, 0.0f, 1.0f);
            Vector3 closest = p0 + seg * t;

            const float dist = (pos - closest).length();

            if (dist <= road.getWidth()/2 + 10) {
                return false;
            }
        }
        return true;
    }
    [[nodiscard]] std::tuple<threepp::Vector3, float, int> getStartPositionAndRotation(const int pos, const int startIndex, const int carsPerRow ) const {
        const auto samples = road.getSamples();
        if (samples.empty()) return {{0,0,0}, 0.0f, 0};

        const int sampleSize = static_cast<int>(samples.size());
        const int gridRow = pos / carsPerRow ;
        const int gridCol = pos % carsPerRow ;

        int stridePerRow = 6;
        int stridePerCol = 2;

        int idx = startIndex - ((gridRow+1) * stridePerRow + gridCol * stridePerCol);

        if (idx < 0) {
            idx += sampleSize;
        } else if (idx >= sampleSize) {
            idx = idx % sampleSize;
        }
        const threepp::Vector3 forward = samples[idx].tangent;
        const threepp::Vector3 lateral(-forward.y, forward.x, 0);

        const float rot = std::atan2(forward.x, forward.y);

        const float sideSpacing = (samples[idx].width / static_cast<float>(carsPerRow)) * 0.75f;

        const float totalRowWidth = static_cast<float>(carsPerRow - 1) * sideSpacing;

        const float centerOffset = totalRowWidth / 2.0f;

        const float rawLateralPosition = static_cast<float>(gridCol) * sideSpacing;
        const float finalLateralOffset = rawLateralPosition - centerOffset;

        threepp::Vector3 finalPos = samples[idx].pos;

        finalPos += lateral * finalLateralOffset;

        return {finalPos, rot, idx};
    }

    void handleMapCollisions(CarLogic& car, const float dt) const {
        const Vector3 prevPos = car.getPreviousPosition();
        const Vector3 pos = car.getPosition();

        const OBB carBox = car.getOrientedBoundingBox();
        const auto corners = carBox.getCorners();
        for (const auto& corner : corners) {
            const int cx = static_cast<int>(corner.x / static_cast<float>(scale_));
            const int cy = static_cast<int>(corner.y / static_cast<float>(scale_));

            if (cx < 0 || cy < 0 || cx >= width_ || cy >= height_) {
                car.setPosition(prevPos);
                car.applyBounce();
            };
        }

        if (isOutsideRoad(pos)) {
            car.setDragMultiplier(4.0f);
            car.setGripMultiplier(0.7f);
            car.setRollingResistanceMultiplier(2.0f);
            car.setMaxSpeedMultiplier(0.6f);
        } else {
            car.setDragMultiplier(1.0f);
            car.setGripMultiplier(1.0f);
            car.setRollingResistanceMultiplier(1.0f);
            car.setMaxSpeedMultiplier(1.0f);
        }
    }

    static void generateOrganicBorder() {}

    MapData snapshot() const {
        MapData data;
        data.mapName = filename_;
        data.roadSamples = road.getSamples();
        data.roadWidth   = road.getWidth();
        data.width = width_;
        data.height = height_;
        data.looped = road.isLooped();
        return data;
    }
};


#endif
