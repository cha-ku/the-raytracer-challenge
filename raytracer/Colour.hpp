//
// Created by chaku on 07/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_COLOUR_HPP
#define THE_RAYTRACER_CHALLENGE_COLOUR_HPP

#include "Utils.hpp"

namespace raytracer {
    struct Colour {
        float r;
        float g;
        float b;

        constexpr auto operator<=>(const Colour& c) const = default;

        constexpr Colour operator*(const float f) const {
            return {.r = f * r, .g = f * g, .b = f * b};
        }
    };

    constexpr Colour operator+(const Colour& c1, const Colour& c2)  {
        return {.r = c1.r + c2.r, .g = c1.g + c2.g, .b = c1.b + c2.b};
    }

    constexpr Colour operator-(const Colour& c1, const Colour& c2)  {
        return {.r = c1.r - c2.r, .g = c1.g - c2.g, .b = c1.b - c2.b};
    }

    constexpr Colour operator*(const Colour& c1, const Colour& c2) {
        return {.r = c1.r * c2.r, .g = c1.g * c2.g, .b = c1.b * c2.b};
    }

    constexpr Colour hadamard_product(const Colour& c1, const Colour& c2) {
        return {.r = c1.r * c2.r, .g = c1.g * c2.g, .b = c1.b * c2.b};
    }

    static constexpr bool areAlmostEqual(const Colour& c1, const Colour& c2) {
        return utils::equal(c1.r, c2.r) && utils::equal(c1.g, c2.g) && utils::equal(c1.b, c2.b);
    }

    // Common colours. In their own namespace so they don't clash with local names like `white`/`black`;
    // use as colours::white, or bring them in with `using namespace raytracer::colours;`
    namespace colours {
        inline constexpr Colour black{.r = 0, .g = 0, .b = 0};
        inline constexpr Colour white{.r = 1, .g = 1, .b = 1};
        inline constexpr Colour red{.r = 1, .g = 0, .b = 0};
        inline constexpr Colour green{.r = 0, .g = 1, .b = 0};
        inline constexpr Colour blue{.r = 0, .g = 0, .b = 1};
        inline constexpr Colour yellow{.r = 1, .g = 1, .b = 0};
        inline constexpr Colour cyan{.r = 0, .g = 1, .b = 1};
        inline constexpr Colour magenta{.r = 1, .g = 0, .b = 1};
        inline constexpr Colour grey{.r = 0.5, .g = 0.5, .b = 0.5};
    }
}

#endif //THE_RAYTRACER_CHALLENGE_COLOUR_HPP
