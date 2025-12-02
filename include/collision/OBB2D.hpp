
#ifndef CARSIM_OBB2D_HPP
#define CARSIM_OBB2D_HPP

#include "threepp/math/Vector2.hpp"
#include <array>
#include <cmath>

// OBB2D: Oriented Bounding Box in 2D
// Initial structure generated with AI assistance, adapted for project needs

struct OBB2D {
    static OBB2D create(const threepp::Vector2& center,
                        const threepp::Vector2& half,
                        const threepp::Vector2& forward);
    [[nodiscard]] std::array<threepp::Vector2, 4> getCorners() const;

    [[nodiscard]] bool containsPoint(const threepp::Vector2& pos) const;

    [[nodiscard]] threepp::Vector2 getRight() const;

    static void projectOBB(const OBB2D& obb, const threepp::Vector2& axis,
                           float& min, float& max);
    [[nodiscard]] bool intersects(const OBB2D& other) const;
private:
    threepp::Vector2 center_;
    threepp::Vector2 half_;
    threepp::Vector2 forward_;
    OBB2D(threepp::Vector2 center, threepp::Vector2 half, threepp::Vector2 forward);
};

#endif // CARSIM_OBB2D_HPP
