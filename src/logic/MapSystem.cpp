#include "logic/MapSystem.hpp"

MapSystem::MapSystem(const std::shared_ptr<MapConfig>& cfg)
        : road(250, true), filename_(cfg->name)
{
    if (!cfg) {
        throw std::runtime_error( cfg->name + "' not found!");
    }
    width_ = cfg->width;
    height_ = cfg->height;

    if (cfg->roadWidth > 0) {
        road.setWidth(cfg->roadWidth);
    }
    const auto rp = cfg->roadPoints;
    if (rp.empty() ) {
        throw std::runtime_error(cfg->name + "does not have any roadpoints");
    }
    for (const auto& p : cfg->roadPoints) {

        road.addPoint({p.x, p.y, p.z});
    }
    road.buildSmoothSamples();
    road.setLooped(cfg->looped);
}

bool MapSystem::isLooped() const {
    return true;
}

int MapSystem::getWidth() const {return width_;}
int MapSystem::getHeight() const {return height_;}

MapSysSnap MapSystem::snapshot() {
    MapSysSnap snap;
    snap.name = filename_;
    snap.roadSamples = road.getSamples();
    snap.roadWidth   = road.getWidth();
    snap.width = width_;
    snap.height = height_;
    snap.looped = road.isLooped();
    return snap;
}
