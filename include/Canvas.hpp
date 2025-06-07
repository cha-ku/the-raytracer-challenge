//
// Created by chaku on 07/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_CANVAS_HPP
#define THE_RAYTRACER_CHALLENGE_CANVAS_HPP

#include <array>
#include <cstdint>
#include <expected>
#include <fstream>
#include "Colour.hpp"

struct Canvas {
    uint32_t width;
    uint32_t height;
    enum class CanvasError {
        invalid_path
    };
    [[nodiscard]] std::expected<bool, CanvasError> write(const std::string& file_path) const {
        std::ofstream out_file(file_path);
        if (!out_file) {
            return std::unexpected(CanvasError::invalid_path);
        }
        out_file << "P3\n" << width << " " << height << "\n255\n";
        return true;
    }
};

#endif //THE_RAYTRACER_CHALLENGE_CANVAS_HPP
