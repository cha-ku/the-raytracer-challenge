//
// Created by chaku on 02/11/2026.
//

#include "Pattern.hpp"
#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "Shape.hpp"
#include <cmath>

namespace raytracer {
    StripedPattern StripedPattern::create(const Colour a, const Colour b) {
        return {.m_colour_a = a, .m_colour_b = b};
    }

    Colour StripedPattern::stripe_at(const StripedPattern &pattern, const Point &point) {
        return std::fmod(std::floor(point.x) , 2) == 0 ? pattern.m_colour_a : pattern.m_colour_b;
    }

    Colour StripedPattern::stripe_at(const StripedPattern &pattern, const Shape &shape, const Point &world_point) {
        const Point object_point{to_object_space(world_point, inverse(shape.m_transform).value())};
        const Point pattern_point{transform(object_point, inverse(pattern.m_transform).value())};
        return stripe_at(pattern, pattern_point);
    }

    Colour colour_at(const ColourOrPattern &colour_or_pattern, const Shape& shape, const Point &point) {
        return std::visit([&point, &shape]<typename T>(const T &value) -> Colour {
            if constexpr (std::same_as<T, StripedPattern>) {
                return StripedPattern::stripe_at(value, shape, point);
            } else {
                return value;
            }
        }, colour_or_pattern);
    }
}
