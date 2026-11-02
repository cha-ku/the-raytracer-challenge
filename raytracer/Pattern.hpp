//
// Created by chaku on 02/11/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_PATTERN_HPP
#define THE_RAYTRACER_CHALLENGE_PATTERN_HPP

#include "Colour.hpp"
#include "Point.hpp"

#include <variant>

namespace raytracer {
    struct StripedPattern {
        Colour a;
        Colour b;

        static StripedPattern create(Colour a, Colour b);
        static Colour stripe_at(const StripedPattern& pattern, const Point& point);

        constexpr auto operator<=>(const StripedPattern &) const = default;
    };

    // What a Material is coloured with: either a single solid Colour, or a pattern whose colour depends on the point
    using ColourOrPattern = std::variant<Colour, StripedPattern>;

    // The colour of a ColourOrPattern at the given point: the Colour itself, or the pattern's colour at that point
    Colour colour_at(const ColourOrPattern &colour_or_pattern, const Point &point);

}

#endif //THE_RAYTRACER_CHALLENGE_PATTERN_HPP
