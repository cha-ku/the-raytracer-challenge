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

    // Shared by every concrete pattern: its transform from object space into pattern space
    struct Pattern {
        Matrix<double> m_transform{Matrix<double>::identity(4)};

        bool operator==(const Pattern &) const = default;
    };

    // StripedPattern
    struct StripedPattern : Pattern {
        Colour m_colour_a{};
        Colour m_colour_b{};

        static StripedPattern create(Colour a, Colour b);
        static Colour stripe_at(const StripedPattern &pattern, const Point &point);

        bool operator==(const StripedPattern &) const = default;
    };

    // StripedPattern pattern_at
    Colour pattern_at(const StripedPattern &pattern, const Point &pattern_point);

    // GradientPattern
    struct GradientPattern : Pattern {
        Colour m_colour_a{};
        Colour m_colour_b{};

        static GradientPattern create(Colour a, Colour b);
        bool operator==(const GradientPattern &) const = default;
    };

    // GradientPattern pattern_at
    Colour pattern_at(const GradientPattern& pattern, const Point &pattern_point);

    struct RingPattern : Pattern {
        Colour m_colour_a{};
        Colour m_colour_b{};

        static RingPattern create(Colour a, Colour b);
        bool operator==(const RingPattern &) const = default;
    };

    // RingPattern pattern_at
    Colour pattern_at(const RingPattern& pattern, const Point &pattern_point);

    // CheckersPattern
    struct CheckersPattern : Pattern {
        Colour m_colour_a{};
        Colour m_colour_b{};

        static CheckersPattern create(Colour a, Colour b);
        bool operator==(const CheckersPattern &) const = default;
    };

    // CheckersPattern pattern_at
    Colour pattern_at(const CheckersPattern& pattern, const Point &pattern_point);

    // TestPattern: colours each point by its pattern-space coordinates (r = x, g = y, b = z), so you can see
    // exactly which point pattern_at received. Used by the tests (via test_pattern()), and handy for
    // visualising object/pattern transforms in a render
    struct TestPattern : Pattern {
        bool operator==(const TestPattern &) const = default;
    };

    // TestPattern pattern_at
    Colour pattern_at(const TestPattern &pattern, const Point &pattern_point);

    // Each entry in this variant needs to define a pattern_at method
    using AnyPattern = std::variant<StripedPattern, GradientPattern, RingPattern, CheckersPattern, TestPattern>;
    // TODO: See BlendedPattern, NestedPattern, RadialGradientPattern, PerturbedPatterns
}

#endif //THE_RAYTRACER_CHALLENGE_PATTERN_HPP
