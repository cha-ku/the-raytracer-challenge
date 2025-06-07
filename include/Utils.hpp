//
// Created by chaku on 26/05/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_UTILS_HPP
#define THE_RAYTRACER_CHALLENGE_UTILS_HPP

#include <cmath>

namespace utils {
    static constexpr auto epsilon = 1e-6f;

    static constexpr bool areAlmostEqual(const float a, const float b) {
        return std::fabs(a - b) < epsilon;
    }
}

#endif //THE_RAYTRACER_CHALLENGE_UTILS_HPP
