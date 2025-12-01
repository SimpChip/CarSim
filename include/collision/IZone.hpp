#ifndef CARSIM_IZONE_HPP
#define CARSIM_IZONE_HPP

#include "threepp/threepp.hpp"

enum class ZoneType { Road, Offroad, Sand};

struct ZoneInfo {
    int id;
    ZoneType type;
    threepp::Vector2 aabbMin;
    threepp::Vector2 aabbMax;
    float reenterMargin = 0.5f;
    int priority = 0;
};

class IZone {
public:
    virtual ~IZone() = default;
    virtual ZoneInfo snapshot() const = 0;
    virtual bool containsPoint(const threepp::Vector2& p) const = 0;
};

#endif //CARSIM_IZONE_HPP