//
// Created by chaku on 07/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_COLOUR_HPP
#define THE_RAYTRACER_CHALLENGE_COLOUR_HPP

#include "Utils.hpp"

struct Colour {
    float r;
    float g;
    float b;

    constexpr auto operator<=>(const Colour& c) const = default;

    constexpr Colour operator*(const float f) const {
        return {f * r, f * g, f* b};
    }
};

constexpr Colour operator+(const Colour& c1, const Colour& c2)  {
    return {c1.r + c2.r, c1.g + c2.g, c1.b + c2.b};
}

constexpr Colour operator-(const Colour& c1, const Colour& c2)  {
    return {c1.r - c2.r, c1.g - c2.g, c1.b - c2.b};
}

constexpr Colour hadamard_product(const Colour& c1, const Colour& c2) {
    return {c1.r * c2.r, c1.g * c2.g, c1.b * c2.b};
}

static constexpr bool areAlmostEqual(const Colour& c1, const Colour& c2) {
    return utils::areAlmostEqual(c1.r, c1.r) && utils::areAlmostEqual(c1.g, c2.g) && utils::areAlmostEqual(c1.b, c2.b);
}

#endif //THE_RAYTRACER_CHALLENGE_COLOUR_HPP
