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
    Canvas canvas{900, 550};

    int ticks = 0;

    // Run simulation
    while ((canvas.height - static_cast<decltype(canvas.height)>(projectile.position.y)) > 0) {
        std::cout << "Tick " << ticks << ": Position (" << projectile.position.x << ", " << projectile.position.y << ", " << projectile.position.z << ")\n";
        projectile = tick(environment, projectile);
        ++ticks;
    }

    std::cout << "Projectile hit the ground after " << ticks << " ticks.\n";
}