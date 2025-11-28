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
    threepp::Vector2 halfExtents;
    threepp::Vector2 forward;

    [[nodiscard]] std::array<threepp::Vector2, 4> getCorners() const {
        const auto right = getRight();
        return {
            center + forward * halfExtents.y + right * halfExtents.x,
            center + forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y + right * halfExtents.x
        };
    }

    bool containsPoint(const threepp::Vector2& p) const {
        threepp::Vector2 d = p - center;

        float projForward = d.dot(forward);
        float projRight   = d.dot({-forward.y, forward.x});

        return std::abs(projForward) <= halfExtents.x &&
               std::abs(projRight)   <= halfExtents.y;
    }


    [[nodiscard]] threepp::Vector2 getRight() const {
        return threepp::Vector2(-forward.y, forward.x).normalize();
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
        threepp::Vector2 axes[4] = { forward, getRight(), other.forward, other.getRight() };
        for (auto& axis : axes) {
            if (axis.lengthSq() < 1e-6f) continue;
            threepp::Vector2 normAxis = axis.normalize();
            float minA, maxA, minB, maxB;
            projectOBB(*this, normAxis, minA, maxA);
            projectOBB(other, normAxis, minB, maxB);
            constexpr float EPS = 1e-3f;
            if (maxA < minB + EPS || maxB < minA + EPS) return false;
        }
        return true;
    }
};

#endif // CARSIM_OBB2D_HPP
