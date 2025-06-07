//
// Created by chaku on 18/05/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_POINT_HPP
#define THE_RAYTRACER_CHALLENGE_POINT_HPP

#include "Vector.hpp"

struct Point {
    float x;
    float y;
    float z;

    constexpr auto operator<=>(const Point& p) const = default;

    constexpr Point operator+(const Point& p) const {
        return {p.x + x, p.y + y, p.z + z};
    }

    constexpr Vector operator-(const Point& p) const {
        return {x - p.x, y - p.y, z - p.z};
    }

    constexpr Point operator-(const Vector& v) const {
        return {x - v.x, y - v.y, z - v.z};
    }
};

#endif //THE_RAYTRACER_CHALLENGE_POINT_HPP
