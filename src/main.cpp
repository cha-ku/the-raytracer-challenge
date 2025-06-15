#include <iostream>
#include <random>
#include <print>
#include "Canvas.hpp"
#include "simulation.hpp"
#include "Utils.hpp"

int main() {
//    simulate();
    raytracer::Canvas canvas{400, 400};
    std::cout << canvas.width << " , " << canvas.height << "\n";
    static constexpr std::string output{"outfile.ppm"};
    for (int j = 0; j < canvas.height; ++j) {
        for (int i = 0; i < canvas.height; ++i) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(0.0f, 1.0f); // Range [0, 1]
            auto colour = raytracer::Colour(dis(gen),dis(gen), dis(gen));
            canvas.write_pixel(i, j, colour);
        }
    }
    const auto retval = raytracer::canvas_to_ppm(canvas, output);
    if (retval.has_value()) {
        std::cout << "Data written to " << output << "\n";
    }
    else if (retval.error() == raytracer::CanvasError::invalid_path){
        std::cout << "Cannot write file " << output << " - invalid path\n";
    }
    return 0;
}
