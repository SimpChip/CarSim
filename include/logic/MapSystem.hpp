//
// Created by Mella on 18.10.2025.
//
#ifndef MAPLOGIC_HPP
#define MAPLOGIC_HPP

#include <fstream>
#include "Road.hpp"
#include "core/ResourceManager.hpp"
#include "types/data.hpp"

class MapSystem {
public:
    Road road;
    int width_{};
    int height_{};
    int scale_ = 1;
    std::string filename_;
    threepp::Vector2 aabbMin_{0,0};
    threepp::Vector2 aabbMax_{0,0};

    explicit MapSystem(const std::string& mapName, const ResourceManager& rm)
        : road(250, true),
          filename_(mapName)
    {
        const auto cfg = rm.getMapConfig(mapName);
        if (!cfg) {
            throw std::runtime_error( mapName + "' not found!");
        }
        width_ = cfg->width;
        height_ = cfg->height;

        if (cfg->roadWidth > 0) {
            road.setWidth(cfg->roadWidth);
        }
        const auto rp = cfg->roadPoints;
        if (rp.empty() ) {
            throw std::runtime_error(mapName + "does not have any roadpoints");
        }
        for (const auto& p : cfg->roadPoints) {

            road.addPoint({p.x, p.y, p.z});
        }
        road.buildSmoothSamples();
        road.setLooped(cfg->looped);
    }

    [[nodiscard]] bool isLooped() const {
        return road.isLooped() ;
    }

    [[nodiscard]] int getWidth() const {return width_;}
    [[nodiscard]] int getHeight() const {return height_;}

    [[nodiscard]] bool isOutsideRoad(const threepp::Vector3& pos) {
        auto roadSamples = road.getSamples();
        for (size_t i = 0; i < roadSamples.size() - 1; ++i) {
            threepp::Vector3 p0 = roadSamples[i].pos;
            threepp::Vector3 p1 = roadSamples[i + 1].pos;
            threepp::Vector3 seg = p1 - p0;
            threepp::Vector3 toPos = pos - p0;

            const float segLenSq = seg.dot(seg);
            if (segLenSq < 1e-6f) continue;

            const float t = std::clamp(toPos.dot(seg) / segLenSq, 0.0f, 1.0f);
            threepp::Vector3 closest = p0 + seg * t;

            const float dist = (pos - closest).length();

            if (dist <= road.getWidth()/2 + 10) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::tuple<threepp::Vector3, float, int> getStartPositionAndRotation(const int pos, const int startIndex, const int carsPerRow ) {
        const auto samples = road.getSamples();
        if (samples.empty()) return {{0,0,0}, 0.0f, 0};

        const int sampleSize = static_cast<int>(samples.size());
        const int gridRow = pos / carsPerRow ;
        const int gridCol = pos % carsPerRow ;

        constexpr int stridePerRow = 6;
        constexpr int stridePerCol = 2;

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

    void computeAABBFromSamples() {
        const auto& samples = road.getSamples();
        if (samples.empty()) {
            aabbMin_ = {0,0};
            aabbMax_ = {0,0};
            return;
        }
        aabbMin_.x = aabbMax_.x = samples.front().pos.x;
        aabbMin_.y = aabbMax_.y = samples.front().pos.y;
        float halfW = road.getWidth() * 0.5f;
        for (const auto& s : samples) {
            aabbMin_.x = std::min(aabbMin_.x, s.pos.x - halfW);
            aabbMin_.y = std::min(aabbMin_.y, s.pos.y - halfW);
            aabbMax_.x = std::max(aabbMax_.x, s.pos.x + halfW);
            aabbMax_.y = std::max(aabbMax_.y, s.pos.y + halfW);
        }
    }

    [[nodiscard]] MapData snapshot() {
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
