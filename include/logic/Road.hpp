#ifndef ROAD_HPP
#define ROAD_HPP

#include <iostream>
#include <vector>
#include <threepp/math/Vector3.hpp>


class Road {
public:
    explicit Road(const int defaultWidth = 250.0f, const bool looped = false) : defaultWidth_(defaultWidth), looped_(looped) {}
    void setLooped(const bool loop) { looped_ = loop; }
    [[nodiscard]] bool isLooped() const {return looped_;}
    void addPoint(const threepp::Vector3& point) {
        points_.push_back(point);
    }

    void setWidth(const int width) {
        defaultWidth_ = width;
    }

    void buildSmoothSamples(const int subdivisions = 200) {
        samples_.clear();
        if (points_.size() < 2) return;

        for (size_t i = 0; i < points_.size(); ++i) {

            const threepp::Vector3 p0 = points_[(i == 0) ? points_.size()-1 : i-1];
            const threepp::Vector3 p1 = points_[i];
            const threepp::Vector3 p2 = points_[(i+1) % points_.size()];
            const threepp::Vector3 p3 = points_[(i+2) % points_.size()];

            for (int j = 0; j < subdivisions; ++j) {
                const float t = static_cast<float>(j) / static_cast<float>(subdivisions);
                const threepp::Vector3 pos = catmullRom(p0, p1, p2, p3, t);
                const threepp::Vector3 tangent = catmullRomTangent(p0, p1, p2, p3, t).normalize();
                threepp::Vector3 right(-tangent.y, tangent.x, 0);
                RoadSample sample{pos, tangent, right.normalize(), defaultWidth_};
                samples_.push_back(sample);
            }
        }
    }

    [[nodiscard]] std::vector<RoadSample>& getSamples() { return samples_; }
    [[nodiscard]] int getWidth() const { return defaultWidth_; }

private:
    std::vector<threepp::Vector3> points_;
    std::vector<RoadSample> samples_;
    int defaultWidth_;
    bool looped_;

    static threepp::Vector3 catmullRom(const threepp::Vector3& p0, const threepp::Vector3& p1,
                   const threepp::Vector3& p2, const threepp::Vector3& p3, const float t) {
        const float t2 = t * t;
        const float t3 = t2 * t;
        return  ((p1*2) +
                       (p2 - p0) * t +
                       (p0*2 - p1*5 + p2*4 - p3) * t2 +
                       (p1*3 - p2*3 + p3 -p0) * t3) * 0.5f;
    }


    static threepp::Vector3 catmullRomTangent(const threepp::Vector3& p0, const threepp::Vector3& p1,
                          const threepp::Vector3& p2, const threepp::Vector3& p3, const float t) {
        const float t2 = t * t;
        return  ((p2-p0) +
                       (p0*2 - p1*5 + p2*4 - p3) * (2*t) +
                       (p1*3 - p2*3 + p3 - p0) * (3*t2)) * 0.5f;
    }
};

#endif //ROAD_HPP