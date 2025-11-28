//
// Created by Mella on 26.11.2025.
//

#ifndef CARSIM_OBB2D_HPP
#define CARSIM_OBB2D_HPP

#include "threepp/math/Vector2.hpp"
#include <array>
#include <cmath>

struct OBB2D {
    threepp::Vector2 center;
    threepp::Vector2 halfExtents; // x = half width, y = half depth
    threepp::Vector2 forward;     // normalized direction along road
    threepp::Vector2 right;       // perpendicular to forward

    [[nodiscard]] std::array<threepp::Vector2, 4> getCorners() const {
        return {
            center + forward * halfExtents.y + right * halfExtents.x,
            center + forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y + right * halfExtents.x
        };
    }

    static void projectOBB(const OBB2D& obb, const threepp::Vector2& axis, float& min, float& max) {
        auto corners = obb.getCorners();
        min = max = corners[0].dot(axis);
        for (size_t i = 1; i < corners.size(); ++i) {
            float projection = corners[i].dot(axis);
            if (projection < min) min = projection;
            if (projection > max) max = projection;
        }
    }

    [[nodiscard]] bool intersects(const OBB2D& other) const {
        threepp::Vector2 axes[4] = { forward, right, other.forward, other.right };
        for (auto& axis : axes) {
            threepp::Vector2 normAxis = axis.normalized();
            float minA, maxA, minB, maxB;
            projectOBB(*this, normAxis, minA, maxA);
            projectOBB(other, normAxis, minB, maxB);
            if (maxA < minB || maxB < minA) return false;
        }
        return true;
    }
};
#endif // CARSIM_OBB2D_HPP
