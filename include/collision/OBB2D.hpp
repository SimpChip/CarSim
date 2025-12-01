
#ifndef CARSIM_OBB2D_HPP
#define CARSIM_OBB2D_HPP

#include "threepp/math/Vector2.hpp"
#include <array>
#include <cmath>
#include <cassert>

struct OBB2D {
    threepp::Vector2 center;
    threepp::Vector2 halfExtents;
    threepp::Vector2 forward;

    static OBB2D create(const threepp::Vector2& c,
                        const threepp::Vector2& he,
                        const threepp::Vector2& fwd) {
        threepp::Vector2 normFwd = fwd;
        if (normFwd.lengthSq() < 1e-12f) normFwd = {0.f, 1.f};
        normFwd.normalize();

        assert(std::abs(normFwd.length() - 1.f) < 1e-4f && "Forward must be normalized");
        assert(he.x > 0 && he.y > 0 && "Half extents must be positive");

        return OBB2D{c, he, normFwd};
    }

    [[nodiscard]] std::array<threepp::Vector2, 4> getCorners() const {
        const auto right = getRight();
        return {
            center + forward * halfExtents.y + right * halfExtents.x,
            center + forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y + right * halfExtents.x
        };
    }

    [[nodiscard]] bool containsPoint(const threepp::Vector2& p) const {
        const threepp::Vector2 d = p - center;
        const auto right = getRight();
        const float projForward = d.dot(forward);
        const float projRight   = d.dot(right);

        return std::abs(projForward) <= halfExtents.y &&
               std::abs(projRight)   <= halfExtents.x;
    }

    [[nodiscard]] threepp::Vector2 getRight() const {
        return { -forward.y, forward.x };
    }

    static void projectOBB(const OBB2D& obb, const threepp::Vector2& axis,
                           float& min, float& max) {
        const auto corners = obb.getCorners();
        min = max = corners[0].dot(axis);
        for (size_t i = 1; i < corners.size(); ++i) {
            const float projection = corners[i].dot(axis);
            if (projection < min) min = projection;
            if (projection > max) max = projection;
        }
    }

    [[nodiscard]] bool intersects(const OBB2D& other) const {
        const threepp::Vector2 axes[4] = { forward, getRight(), other.forward, other.getRight() };

        constexpr float AXIS_EPS = 1e-6f;
        constexpr float SEP_EPS  = 1e-3f;

        for (const auto& a : axes) {
            const float len2 = a.lengthSq();
            if (len2 < AXIS_EPS) continue;

            threepp::Vector2 axis = a;
            axis.normalize();

            float minA, maxA, minB, maxB;
            projectOBB(*this, axis, minA, maxA);
            projectOBB(other, axis, minB, maxB);

            if (maxA < minB + SEP_EPS || maxB < minA + SEP_EPS) {
                return false;
            }
        }
        return true;
    }
};

#endif // CARSIM_OBB2D_HPP
