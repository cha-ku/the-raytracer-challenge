//
// Created by chaku on 15/06/25.
//

#include "Canvas.hpp"

namespace raytracer {
    void Canvas::write_pixel(uint32_t pix_w, uint32_t pix_h, const Colour& colour) {
        storage[pix_w * pix_h] = {static_cast<uint8_t>(colour.r * 255), static_cast<uint8_t>(colour.g * 255), static_cast<uint8_t>(colour.b * 255)};
    }

    std::expected<bool, CanvasError> canvas_to_ppm(const Canvas& canvas, const std::string& file_path) {
        std::ofstream out_file(file_path, std::ios::trunc);
        if (!out_file) {
            return std::unexpected(CanvasError::invalid_path);
        }
        const std::string header{"P3\n" + std::to_string(canvas.width) + " " + std::to_string(canvas.height) + "\n255\n"};
        out_file << header;
        for (const auto& row : canvas.storage) {
            const std::vector<std::string> strings =
                    row | std::views::transform([](int n) { return std::to_string(n); })
                    | std::ranges::to<std::vector<std::string>>();
            auto out_str = std::ranges::to<std::string>(std::views::join_with(strings, ' '));
            /*
            if (out_str.size() > 70) {
                out_file << out_str.substr(0,  70);
                out_file << "\n";
                out_file << out_str.substr(71);
            }
            else {
                out_file << out_str;
            }
            */
            out_file << out_str;
            out_file << "\n";
        }
        return true;
    }
}

