//
// Created by chaku on 05/04/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_CAMERA_HPP
#define THE_RAYTRACER_CHALLENGE_CAMERA_HPP

#include <cstdint>
#include <numbers>

#include "Matrix.hpp"

namespace raytracer {
    struct Camera {
        uint32_t hsize{160};
        uint32_t vsize{120};
        double field_of_view{std::numbers::pi_v<double>/2};
        Container<double> transform{Container<double>::identity(4)};

        double half_view{std::tan(field_of_view / 2)};
        double aspect{static_cast<double>(hsize) / vsize};
        double half_width{aspect >= 1.0 ? half_view : half_view * aspect};
        double half_height{aspect >= 1.0 ? half_view / aspect : half_view};
        double pixel_size{(half_width * 2) / hsize};
    };
};

#endif //THE_RAYTRACER_CHALLENGE_CAMERA_HPP