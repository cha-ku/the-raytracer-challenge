#include <iostream>
#include <random>
#include <print>
#include <string_view>
#include "Canvas.hpp"
#include "Simulation.hpp"
#include "Utils.hpp"


// raytracer [model.obj | clock-tower]: renders the OBJ model or the CSG clock tower if one is given, the group
// reflections scene otherwise
int main(const int argc, char *argv[]) {
    // simulate_projectile();
    // simulate_clock();
    // simulate_sphere();
    // simulate_material_sphere();
    // simulate_multiple_spheres();
    if (argc < 2) {
        simulate_clock_tower();
        return 0;
    }
    try {
        simulate_model(argv[1]);
    } catch (const std::exception &e) {
        // e.g. the file can't be opened
        std::println(stderr, "{}", e.what());
        return 1;
    }
    return 0;
}
