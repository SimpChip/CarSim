#ifndef ROAD_HPP
#define ROAD_HPP

#include "threepp/threepp.hpp"
#include <vector>
#include <random>
#include "data/types.h"

using Vector3 = threepp::Vector3;

class RoadLogic {
public:
    explicit RoadLogic(const float defaultWidth = 250.0f, const bool looped = false) : defaultWidth_(defaultWidth), looped_(looped) {}
    void setLooped(const bool loop) { looped_ = loop; }
    [[nodiscard]] bool isLooped() const {return looped_;}
    void addPoint(const Vector3& point) {
        points_.push_back(point);
    }

    void buildSmoothSamples(const int subdivisions = 100) {
        samples_.clear();
        if (points_.size() < 2) return;

        for (size_t i = 0; i < points_.size(); ++i) {

            const Vector3 p0 = points_[(i == 0) ? points_.size()-1 : i-1];
            const Vector3 p1 = points_[i];
            const Vector3 p2 = points_[(i+1) % points_.size()];
            const Vector3 p3 = points_[(i+2) % points_.size()];

            for (int j = 0; j < subdivisions; ++j) {
                const float t = static_cast<float>(j) / static_cast<float>(subdivisions);
                const Vector3 pos = catmullRom(p0, p1, p2, p3, t);
                const Vector3 tangent = catmullRomTangent(p0, p1, p2, p3, t).normalize();
                Vector3 right(-tangent.y, tangent.x, 0);
                RoadSample sample{pos, tangent, right.normalize(), defaultWidth_};
                samples_.push_back(sample);
            }
        }
    }

    [[nodiscard]] const std::vector<RoadSample>& getSamples() const { return samples_; }
    [[nodiscard]] float getWidth() const { return defaultWidth_; }

private:
    std::vector<Vector3> points_;
    std::vector<RoadSample> samples_;
    float defaultWidth_;
    bool looped_;

    static Vector3 catmullRom(const Vector3& p0, const Vector3& p1,
                   const Vector3& p2, const Vector3& p3, const float t) {
        const float t2 = t * t;
        const float t3 = t2 * t;
        return  ((p1*2) +
                       (p2 - p0) * t +
                       (p0*2 - p1*5 + p2*4 - p3) * t2 +
                       (p1*3 - p2*3 + p3 -p0) * t3) * 0.5f;
    }

    static Vector3 catmullRomTangent(const Vector3& p0, const Vector3& p1,
                          const Vector3& p2, const Vector3& p3, const float t) {
        const float t2 = t * t;
        return  ((p2-p0) +
                       (p0*2 - p1*5 + p2*4 - p3) * (2*t) +
                       (p1*3 - p2*3 + p3 - p0) * (3*t2)) * 0.5f;
    }
};

#endif //ROAD_HPP