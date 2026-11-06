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
            return {.x = x + v.x, .y = y + v.y, .z = z + v.z};
        }

        constexpr Vector operator-(const Vector &v) const {
            return {.x = x - v.x, .y = y - v.y, .z = z - v.z};
        }

        constexpr Vector operator-() const {
            return {.x = -x, .y = -y, .z = -z};
        }

        constexpr Vector operator*(const float &m) const {
            return {.x = m * x, .y = m * y, .z = m * z};
        }

        constexpr Vector operator/(const float &m) const {
            return {.x = 1 / m * x, .y = 1 / m * y, .z = 1 / m * z};
        }

        static constexpr float magnitude(const Vector &v) {
            return std::sqrt(utils::square(v.x) + utils::square(v.y) + utils::square(v.z));
        }

        static constexpr Vector normalize(const Vector &v) {
            const auto mag = magnitude(v);
            return {.x = v.x / mag, .y = v.y / mag, .z = v.z / mag};
        }

        static constexpr bool areAlmostEqual(const Vector &v, const Vector &w) {
            return utils::equal(v.x, w.x) && utils::equal(v.y, w.y) && utils::equal(v.z, w.z);
        }

        static constexpr float dot(const Vector &v, const Vector &w) {
            return v.x * w.x + v.y * w.y + v.z * w.z;
        }

        static constexpr Vector cross(const Vector &v, const Vector &w) {
            return {
                .x = v.y * w.z - v.z * w.y,
                .y = v.z * w.x - v.x * w.z,
                .z = v.x * w.y - v.y * w.x
            };
        }

        static constexpr Vector reflect(const Vector& in, const Vector& normal) {
            return {in - normal * 2 * dot(in, normal)};
        }
    };
}

#endif //THE_RAYTRACER_CHALLENGE_VECTOR_HPP
