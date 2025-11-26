//
// Created by Mella on 26.11.2025.
//

#ifndef CARSIM_OBB_HPP
#define CARSIM_OBB_HPP

#include "threepp/threepp.hpp"

struct OBB {
    threepp::Vector3 center;
    threepp::Vector3 halfExtents;
    threepp::Vector3 forward;
    threepp::Vector3 right;

    [[nodiscard]] bool containsPoint(const threepp::Vector3& point) const {
        const threepp::Vector3 local = point - center;
        const float xProj = local.dot(right);
        const float yProj = local.dot(forward);
        return std::abs(xProj) <= halfExtents.x && std::abs(yProj) <= halfExtents.y;
    }

    static void projectOBB(const OBB& obb, const threepp::Vector3& axis, float& min, float& max) {
        const auto corners = obb.getCorners();
        min = max = corners[0].dot(axis);
        for (size_t i = 1; i < 4; ++i) {
            const float projection = corners[i].dot(axis);
            if (projection < min) min = projection;
            if (projection > max) max = projection;
        }
    }

    [[nodiscard]] bool intersects(const OBB& other) const {
        threepp::Vector3 axes[4] = {
            this->forward,
            this->right,
            other.forward,
            other.right
        };
        for (auto& axis : axes) {
            const threepp::Vector3 normalizedAxis = axis.normalize();

            float minA, maxA;
            projectOBB(*this, normalizedAxis, minA, maxA);

            float minB, maxB;
            projectOBB(other, normalizedAxis, minB, maxB);
            if (maxA < minB || maxB < minA) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::array<threepp::Vector3, 4> getCorners() const {
        return {
            center + forward * halfExtents.y + right * halfExtents.x,
            center + forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y - right * halfExtents.x,
            center - forward * halfExtents.y + right * halfExtents.x
            };
    }
};

#endif //CARSIM_OBB_HPP