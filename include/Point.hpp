//
// Created by chaku on 18/05/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_POINT_HPP
#define THE_RAYTRACER_CHALLENGE_POINT_HPP

#include "Vector.hpp"

namespace raytracer {
    struct Point {
        float x;
        float y;
        float z;

        constexpr auto operator<=>(const Point& p) const = default;

        constexpr Vector operator-(const Point& p) const {
            return {x - p.x, y - p.y, z - p.z};
        }

        constexpr Point operator-(const Vector& v) const {
            return {x - v.x, y - v.y, z - v.z};
        }
    };

    constexpr Point operator+(const Point& p, const Point& q) {
        return {p.x + q.x, p.y + q.y, p.z + q.z};
    }

    constexpr Point operator+(const Point& p, const Vector& v) {
        return {p.x + v.x, p.y + v.y, p.z + v.z};
    }
}

#endif //THE_RAYTRACER_CHALLENGE_POINT_HPP
