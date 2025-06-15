//
// Created by chaku on 26/05/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_UTILS_HPP
#define THE_RAYTRACER_CHALLENGE_UTILS_HPP

#include <cmath>

namespace utils {
    static constexpr bool equal(const float a, const float b, const float epsilon = 1e-6f) {
        return std::fabs(a - b) < epsilon;
    }
}

#endif //THE_RAYTRACER_CHALLENGE_UTILS_HPP
