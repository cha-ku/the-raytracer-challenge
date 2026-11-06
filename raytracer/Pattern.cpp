//
// Created by chaku on 02/11/2026.
//

#include "Pattern.hpp"
#include "Utils.hpp"

#include <cmath>

namespace raytracer {
    StripedPattern StripedPattern::create(const Colour a, const Colour b) {
        return {.m_colour_a = a, .m_colour_b = b};
    }

    Colour StripedPattern::stripe_at(const StripedPattern &pattern, const Point &point) {
        return std::fmod(std::floor(point.x), 2) == 0 ? pattern.m_colour_a : pattern.m_colour_b;
    }

    Colour pattern_at(const StripedPattern &pattern, const Point &pattern_point) {
        return StripedPattern::stripe_at(pattern, pattern_point);
    }

    GradientPattern GradientPattern::create(const Colour a, const Colour b) {
        return {.m_colour_a = a, .m_colour_b = b};
    }

    Colour pattern_at(const GradientPattern &pattern, const Point &pattern_point) {
        const auto distance = pattern.m_colour_b - pattern.m_colour_a;
        const auto fraction = pattern_point.x - std::floor(pattern_point.x);
        return pattern.m_colour_a + distance * fraction;
    }

    RingPattern RingPattern::create(const Colour a, const Colour b) {
        return {.m_colour_a = a, .m_colour_b = b};
    }

    Colour pattern_at(const RingPattern &pattern, const Point &pattern_point) {
        const auto distance_from_origin = std::sqrtf(
            utils::square(pattern_point.x) + utils::square(pattern_point.z));
        return std::fmod(std::floor(distance_from_origin), 2) == 0 ? pattern.m_colour_a : pattern.m_colour_b;
    }

    CheckersPattern CheckersPattern::create(Colour a, Colour b) {
        return {.m_colour_a = a, .m_colour_b = b};
    }

    Colour pattern_at(const CheckersPattern &pattern, const Point &pattern_point) {
        return std::fmod(std::floor(pattern_point.x) + std::floor(pattern_point.y) + std::floor(pattern_point.z), 2) ==
               0
                   ? pattern.m_colour_a
                   : pattern.m_colour_b;
    }

    Colour pattern_at(const TestPattern &, const Point &pattern_point) {
        return Colour{.r = pattern_point.x, .g = pattern_point.y, .b = pattern_point.z};
    }
}
