//
// Created by chaku on 02/11/2026.
//

#include "Pattern.hpp"

#include <cmath>

namespace raytracer {
    StripedPattern StripedPattern::create(const Colour a, const Colour b) {
        return {.m_colour_a = a, .m_colour_b = b};
    }

    Colour StripedPattern::stripe_at(const StripedPattern &pattern, const Point &point) {
        return std::fmod(std::floor(point.x) , 2) == 0 ? pattern.m_colour_a : pattern.m_colour_b;
    }

    Colour pattern_at(const StripedPattern &pattern, const Point &pattern_point) {
        return StripedPattern::stripe_at(pattern, pattern_point);
    }
}
