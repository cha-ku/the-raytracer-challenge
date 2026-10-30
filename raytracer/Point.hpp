//
// Created by chaku on 18/05/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_POINT_HPP
#define THE_RAYTRACER_CHALLENGE_POINT_HPP

#include "Vector.hpp"

namespace raytracer {
    struct Point {
        float x{0};
        float y{0};
        float z{0};
        float w{1};

        constexpr auto operator<=>(const Point& p) const = default;

        constexpr Vector operator-(const Point& p) const {
            return {.x = x - p.x, .y = y - p.y, .z = z - p.z};
        }

        constexpr Point operator-(const Vector& v) const {
            return {.x = x - v.x, .y = y - v.y, .z = z - v.z};
        }
    };

    constexpr Point operator+(const Point& p, const Point& q) {
        return {.x = p.x + q.x, .y = p.y + q.y, .z = p.z + q.z};
    }

    constexpr Point operator+(const Point& p, const Vector& v) {
        return {.x = p.x + v.x, .y = p.y + v.y, .z = p.z + v.z};
    }
}

#endif //THE_RAYTRACER_CHALLENGE_POINT_HPP
