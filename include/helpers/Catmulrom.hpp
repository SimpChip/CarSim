#ifndef CARSIM_HELPERS_HPP
#define CARSIM_HELPERS_HPP

#include "threepp/math/Vector3.hpp"


// CatmulRom
// Initial structure generated with AI assistance, adapted for project needs

static threepp::Vector3 catmullRom(const threepp::Vector3& p0, const threepp::Vector3& p1,
               const threepp::Vector3& p2, const threepp::Vector3& p3, const float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    return  ((p1*2) +
                   (p2 - p0) * t +
                   (p0*2 - p1*5 + p2*4 - p3) * t2 +
                   (p1*3 - p2*3 + p3 -p0) * t3) * 0.5f;
}

// CatmulRomTangent
// Initial structure generated with AI assistance, adapted for project needs

static threepp::Vector3 catmullRomTangent(const threepp::Vector3& p0, const threepp::Vector3& p1,
                      const threepp::Vector3& p2, const threepp::Vector3& p3, const float t) {
    const float t2 = t * t;
    return  ((p2-p0) +
                   (p0*2 - p1*5 + p2*4 - p3) * (2*t) +
                   (p1*3 - p2*3 + p3 - p0) * (3*t2)) * 0.5f;
}

#endif //CARSIM_HELPERS_HPP