#ifndef MAPLOGIC_HPP
#define MAPLOGIC_HPP

#include "../entities/Road.hpp"
#include "../helpers/ResourceManager.hpp"
#include "types/data.hpp"
#include "types/gameSnapshots.hpp"

class MapSystem {
public:
    Road road;
    explicit MapSystem(const std::shared_ptr<MapConfig>& cfg);

    [[nodiscard]] bool isLooped() const;
    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
    [[nodiscard]] MapSysSnap snapshot();

private:
    int width_{};
    int height_{};
    int scale_ = 1;
    std::string filename_;
};


#endif
