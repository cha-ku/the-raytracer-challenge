//
// Created by chaku on 07/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_CANVAS_HPP
#define THE_RAYTRACER_CHALLENGE_CANVAS_HPP

#include <cstdint>
#include <vector>
#include <expected>
#include <ranges>
#include "Colour.hpp"

namespace raytracer {
    enum class PixelError {
        bad_dimensions
    };
    enum class CanvasError {
        invalid_path
    };
    struct Canvas {
        uint32_t width{0};
        uint32_t height{0};
        std::vector<std::vector<uint8_t>> storage{width * height, {0, 0, 0}};

        explicit constexpr Canvas(uint32_t h, uint32_t w) : height(h), width(w) {};
        void write_pixel(uint32_t pix_w, uint32_t pix_h, const Colour& colour);
    };
    std::expected<bool, CanvasError> canvas_to_ppm(const Canvas& canvas, const std::string& file_path);
}

#endif //THE_RAYTRACER_CHALLENGE_CANVAS_HPP
