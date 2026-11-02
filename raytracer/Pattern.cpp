//
// Created by chaku on 02/11/2026.
//

#include "Pattern.hpp"
#include <cmath>

namespace raytracer {
    StripedPattern StripedPattern::create(const Colour a, const Colour b) {
        return {.a = a, .b = b};
    }

    Colour StripedPattern::stripe_at(const StripedPattern &pattern, const Point &point) {
        return std::fmod(std::floor(point.x) , 2) == 0 ? pattern.a : pattern.b;
    }

    Colour colour_at(const ColourOrPattern &colour_or_pattern, const Point &point) {
        return std::visit([&point]<typename T>(const T &value) -> Colour {
            if constexpr (std::same_as<T, StripedPattern>) {
                return StripedPattern::stripe_at(value, point);
            } else {
                return value;
            }
        }, colour_or_pattern);
    }
}
