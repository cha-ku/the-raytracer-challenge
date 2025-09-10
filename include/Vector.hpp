//
// Created by chaku on 18/05/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_VECTOR_HPP
#define THE_RAYTRACER_CHALLENGE_VECTOR_HPP

#include "Utils.hpp"

namespace raytracer {
    struct Vector {
        float x;
        float y;
        float z;

        constexpr auto operator<=>(const Vector &v) const = default;

        constexpr Vector operator+(const Vector &v) const {
            return {x + v.x, y + v.y, z + v.z};
        }

        constexpr Vector operator-(const Vector &v) const {
            return {x - v.x, y - v.y, z - v.z};
        }

        constexpr Vector operator-() const {
            return {-x, -y, -z};
        }

        constexpr Vector operator*(const float &m) const {
            return {m * x, m * y, m * z};
        }

        constexpr Vector operator/(const float &m) const {
            return {1 / m * x, 1 / m * y, 1 / m * z};
        }

        static constexpr float magnitude(const Vector &v) {
            return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        }

        static constexpr Vector normalize(const Vector &v) {
            const auto mag = magnitude(v);
            return {v.x / mag, v.y / mag, v.z / mag};
        }

        static constexpr bool areAlmostEqual(const Vector &v, const Vector &w) {
            return utils::equal(v.x, w.x) && utils::equal(v.y, w.y) && utils::equal(v.z, w.z);
        }

        static constexpr float dot(const Vector &v, const Vector &w) {
            return v.x * w.x + v.y * w.y + v.z * w.z;
        }

        static constexpr Vector cross(const Vector &v, const Vector &w) {
            return {
                v.y * w.z - v.z * w.y,
                v.z * w.x - v.x * w.z,
                v.x * w.y - v.y * w.x
            };
        }
    };
}

#endif //THE_RAYTRACER_CHALLENGE_VECTOR_HPP
