//
// Created by chaku on 14/06/25.
//

#include <iostream>
#include "Point.hpp"
#include "Vector.hpp"
#include "simulation.hpp"
#include "Canvas.hpp"

// Projectile structure
struct Projectile {
    raytracer::Point position;
    raytracer::Vector velocity;
};

// Environment structure
struct Environment {
    raytracer::Vector gravity;
    raytracer::Vector wind;
};

// Tick function
Projectile tick(const Environment& env, const Projectile& proj) {
    raytracer::Point new_position = proj.position + proj.velocity;
    raytracer::Vector new_velocity = proj.velocity + env.gravity + env.wind;
    return {new_position, new_velocity};
}


void simulate() {
    using namespace raytracer;
    // Initialize projectile and environment
    Point start{0, 1, 0};
    Vector velocity{Vector::normalize({1, 1.8, 0}) * 11.25};
    Projectile projectile {start, velocity};
    Environment environment{Vector(0, -0.1, 0), Vector(-0.01, 0, 0)};
    Canvas canvas{900, 900};

    int ticks = 0;

    const auto within_bounds = [](const Canvas& canvas, const Projectile& projectile){
        return static_cast<decltype(canvas.height)>(projectile.position.y) < canvas.height
        &&  static_cast<decltype(canvas.width)>(projectile.position.x) < canvas.width;
    };

    // Run simulation
    while (within_bounds(canvas, projectile)) {
        const auto height = static_cast<float>(canvas.height);
        canvas.write_pixel(static_cast<uint32_t>(projectile.position.x), static_cast<uint32_t>(height - projectile.position.y), Colour(0.66, 0.11, 0.3));
        projectile = tick(environment, projectile);
        ++ticks;
    }
    std::cout << "Projectile hit the ground after " << ticks << " ticks.\n";
    static constexpr std::string output{"projectile.ppm"};
    const auto& retval = raytracer::canvas_to_ppm(canvas, output);
    if (retval.has_value()) {
        std::cout << "Data written to " << output << "\n";
    }
    else if (retval.error() == raytracer::CanvasError::invalid_path){
        std::cout << "Cannot write file " << output << " - invalid path\n";
    }

}