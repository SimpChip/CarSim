#ifndef CARSIM_MAPCREATORLOGIC_HPP
#define CARSIM_MAPCREATORLOGIC_HPP

#include <vector>
#include <threepp/math/Vector3.hpp>
#include <chrono>

class MapMakerSystem {
public:
    explicit MapMakerSystem(int mapSize = 100)
        : mapSize_(mapSize), looped_(true),
          lastUpdate_(std::chrono::steady_clock::now()) {}

    void addPoint(const threepp::Vector3& pos) {
        pointPositions_.push_back(pos);
    }

    void undoLastPoint() {
        if (!pointPositions_.empty()) {
            pointPositions_.pop_back();
        }
    }

    void clearPoints() {
        pointPositions_.clear();
    }

    void setMapSize(const int newSize) {
        const auto now = std::chrono::steady_clock::now();
        if (now - lastUpdate_ > std::chrono::milliseconds(1000)) {
            mapSize_ = newSize;
            lastUpdate_ = now;
            clearPoints();
        }
    }
    static void saveMap() {
        std::cout << "Saving map..." << std::endl;
    }
    void setLooped(const bool value) { looped_ = value; }


    [[nodiscard]] int getMapSize() const { return mapSize_; }
    [[nodiscard]] bool isLooped() const { return looped_; }
    [[nodiscard]] const std::vector<threepp::Vector3>& getPoints() const { return pointPositions_; }

    void update(float dt) {}


    [[nodiscard]] MapMakerSnapshot snapshot() const {
        MapMakerSnapshot data;
        data.points = pointPositions_;
        data.mapSize = mapSize_;
        data.looped = looped_;
        return data;
    }

private:
    int mapSize_;
    bool looped_;
    std::vector<threepp::Vector3> pointPositions_;
    std::chrono::steady_clock::time_point lastUpdate_;
};

#endif // CARSIM_MAPCREATORLOGIC_HPP