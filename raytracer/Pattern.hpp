//
// Created by chaku on 02/11/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_PATTERN_HPP
#define THE_RAYTRACER_CHALLENGE_PATTERN_HPP

#include "Colour.hpp"
#include "MatrixImpl.hpp"
#include "Point.hpp"

#include <concepts>
#include <variant>

namespace raytracer {
    template<typename P>
    concept Patternable = requires(const P &pattern, const Point &pattern_point) {
        { pattern_at(pattern, pattern_point) } -> std::same_as<Colour>;
    };

    template<Patternable P>
    Colour pattern_at_shape(const P &pattern, const Matrix<double> &object_transform, const Point &world_point) {
        const auto object_transform_inverse{inverse(object_transform)};
        const auto pattern_transform_inverse{inverse(pattern.m_transform)};
        if (!object_transform_inverse.has_value() || !pattern_transform_inverse.has_value()) {
            return {};
        }
        const Point object_point{transform(world_point, object_transform_inverse.value())};
        const Point pattern_point{transform(object_point, pattern_transform_inverse.value())};
        return pattern_at(pattern, pattern_point);
    }

    // Shared by every concrete pattern: its transform from object space into pattern space
    struct Pattern {
        Matrix<double> m_transform{Matrix<double>::identity(4)};

        bool operator==(const Pattern &) const = default;
    };

    struct StripedPattern : Pattern {
        Colour m_colour_a{};
        Colour m_colour_b{};

        static StripedPattern create(Colour a, Colour b);
        static Colour stripe_at(const StripedPattern &pattern, const Point &point);

        bool operator==(const StripedPattern &) const = default;
    };

    // Each entry in this variant needs to define a pattern_at method
    using AnyPattern = std::variant<StripedPattern>;

    // StripedPattern pattern_at
    Colour pattern_at(const StripedPattern &pattern, const Point &pattern_point);
}

#endif //THE_RAYTRACER_CHALLENGE_PATTERN_HPP
