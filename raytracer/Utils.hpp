//
// Created by chaku on 26/05/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_UTILS_HPP
#define THE_RAYTRACER_CHALLENGE_UTILS_HPP

#include <cmath>
#include <vector>

namespace utils {
    static constexpr float EPSILON = 1e-2f;

    static constexpr bool equal(const float a, const float b, const float epsilon = EPSILON) {
        return std::fabs(a - b) < epsilon;
    }

    static constexpr bool is_almost_equal(const std::vector<double> &a, const std::vector<double> &b,
                                          const double epsilon = EPSILON) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::abs(a[i] - b[i]) > epsilon) return false;
        }
        return true;
    }
}

#endif //THE_RAYTRACER_CHALLENGE_UTILS_HPP
