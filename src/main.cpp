#include <iostream>
#include <random>
#include <print>
#include "Canvas.hpp"
#include "simulation.hpp"
#include "Utils.hpp"

void test() {
    raytracer::Canvas c(16, 16);
    const raytracer::Colour color(0.66, 0.33, 0.33);
    for (int h = 0; h < c.height; ++h) {
        for (int w = 0; w < c.width; ++w) {
            if (h < 10) {
                c.write_pixel(w, h, color);
            }
        }
    }
    [[maybe_unused]] auto ret = raytracer::canvas_to_ppm(c, "testfile.ppm");
}

int main() {
    // simulate_projectile();
    simulate_clock();
    //test();
    return 0;
}
