#include "collision/OBB2D.hpp"
#include "cassert"

// OBB2D: Oriented Bounding Box in 2D
// Initial structure generated with AI assistance, adapted for project needs

OBB2D::OBB2D(
    const threepp::Vector2 center,
    const threepp::Vector2 half,
    const threepp::Vector2 forward)
:
    center_(center),
    half_(half),
    forward_(forward)
{}

OBB2D OBB2D::create(const threepp::Vector2& center,
                        const threepp::Vector2& half,
                        const threepp::Vector2& forward) {
    threepp::Vector2 normFwd = forward;
    if (normFwd.lengthSq() < 1e-12f) normFwd = {0.f, 1.f};
    normFwd.normalize();

    assert(std::abs(normFwd.length() - 1.f) < 1e-4f && "Forward must be normalized");
    assert(half.x > 0 && half.y > 0 && "Half extents must be positive");

    return OBB2D{center, half, normFwd};
}

std::array<threepp::Vector2, 4> OBB2D::getCorners() const {
    const auto right = getRight();
    return {
        center_ + forward_ * half_.y + right * half_.x,
        center_ + forward_ * half_.y - right * half_.x,
        center_ - forward_ * half_.y - right * half_.x,
        center_ - forward_ * half_.y + right * half_.x
    };
}

bool OBB2D::containsPoint(const threepp::Vector2& pos) const {
    const threepp::Vector2 d = pos - center_;
    const auto right = getRight();
    const float projForward = d.dot(forward_);
    const float projRight   = d.dot(right);

    return std::abs(projForward) <= half_.y &&
           std::abs(projRight)   <= half_.x;
}

[[nodiscard]] threepp::Vector2 OBB2D::getRight() const {
    return { -forward_.y, forward_.x };
}

void OBB2D::projectOBB(const OBB2D& obb, const threepp::Vector2& axis,
                       float& min, float& max) {
    const auto corners = obb.getCorners();
    min = max = corners[0].dot(axis);
    for (size_t i = 1; i < corners.size(); ++i) {
        const float projection = corners[i].dot(axis);
        if (projection < min) min = projection;
        if (projection > max) max = projection;
    }
}

bool OBB2D::intersects(const OBB2D& other) const {
    const threepp::Vector2 axes[4] = { forward_, getRight(), other.forward_, other.getRight() };

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

