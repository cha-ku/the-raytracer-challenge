//
// Created by chaku on 14/06/25.
//

// ReSharper disable CppTooWideScope
#include <iostream>
#include "Point.hpp"
#include "Vector.hpp"
#include "simulation.hpp"
#include "Canvas.hpp"
#include <numbers>

#include "Intersect.hpp"
#include "MatrixImpl.hpp"

// Projectile structure
struct Projectile {
    raytracer::Point position{};
    raytracer::Vector velocity{};
};

// Environment structure
struct Environment {
    raytracer::Vector gravity;
    raytracer::Vector wind;
};

// Tick function
Projectile tick(const Environment &env, const Projectile &proj) {
    raytracer::Point new_position = proj.position + proj.velocity;
    raytracer::Vector new_velocity = proj.velocity + env.gravity + env.wind;
    return {new_position, new_velocity};
}

void save_canvas(const raytracer::Canvas& canvas, const std::string& filename) {
    const auto& retval = raytracer::canvas_to_ppm(canvas, filename);
    if (retval.has_value()) {
        std::cout << "Data written to " << filename << "\n";
    } else if (retval.error() == raytracer::CanvasError::invalid_path) {
        std::cout << "Cannot write file " << filename << " - invalid path\n";
    }
}


void simulate_projectile() {
    using namespace raytracer;
    // Initialize projectile and environment
    Point start{0, 1, 0};
    Vector velocity{Vector::normalize({1, 1.8, 0}) * 11.25};
    Projectile projectile{start, velocity};
    Environment environment{Vector(0, -0.1, 0), Vector(-0.01, 0, 0)};
    Canvas canvas{900, 900};

    int ticks = 0;

    const auto within_bounds = [](const Canvas &canvas, const Projectile &projectile) {
        return static_cast<decltype(canvas.height)>(projectile.position.y) < canvas.height
               && static_cast<decltype(canvas.width)>(projectile.position.x) < canvas.width;
    };

    // Run simulation
    while (within_bounds(canvas, projectile)) {
        const auto height = static_cast<float>(canvas.height);
        canvas.write_pixel(static_cast<uint32_t>(projectile.position.x),
                           static_cast<uint32_t>(height - projectile.position.y), Colour(0.66, 0.11, 0.3));
        projectile = tick(environment, projectile);
        ++ticks;
    }
    std::cout << "Projectile hit the ground after " << ticks << " ticks.\n";
    save_canvas(canvas, "projectile.ppm");
}

void simulate_clock() {
    using namespace raytracer;
    Canvas canvas{256, 256};
    const Point centre{static_cast<float>(canvas.width)/2.f, 0, static_cast<float>(canvas.height)/2};
    canvas.write_pixel(static_cast<uint32_t>(centre.x), static_cast<uint32_t>(centre.z),  Colour(1, 0, 0));
    const float radius{3/8.f * static_cast<float>(canvas.width)};
    // 12 o'clock position relative to origin
    const auto twelve_oclock = make_container({0, 0, -radius});

    for (int i = 0; i < 12; ++i) {
        const auto rad = static_cast<float>(i) * std::numbers::pi_v<float>/6.f;
        const auto hour{rotation_y(rad)};
        try {
            // Rotate the point around origin
            const auto rotated = multiply(hour, twelve_oclock);
            // Translate to canvas center
            auto x = rotated.m_data[0] + centre.x;
            auto z = rotated.m_data[2] + centre.z;

            if (x >= 0.f && x < static_cast<float>(canvas.width) &&
                z >= 0.f && z < static_cast<float>(canvas.height)) {
                canvas.write_pixel(static_cast<uint32_t>(x), static_cast<uint32_t>(z),
                    Colour(1, 1, 1));
            }
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Error at hour " << i << ": " << e.what() << "\n";
        }
    }
    save_canvas(canvas, "clock.ppm");
}

void simulate_sphere() {
    using namespace raytracer;
    constexpr auto canvas_pixels{256u};
    constexpr auto wall_size{7.f};
    constexpr auto pixel_size{wall_size/canvas_pixels};
    constexpr auto half{wall_size / 2};
    Canvas canvas{canvas_pixels, canvas_pixels};
    constexpr Colour colour{.r=1.f};
    Sphere shape = Sphere::make_sphere();
    // shrink it along the y axis
    set_transform(shape, scale<double>(1, 0.5, 1));
    // shrink it along the x axis
    set_transform(shape, scale<double>(0.5, 1, 1));
    // shrink it, and rotate it
    set_transform(shape, multiply(rotation_z(std::numbers::pi/4),  scale<double>(0.5, 1, 1)));
    // shrink it, and skew it
    set_transform(shape, multiply(shearing(1, 0, 0, 0, 0, 0), scale<double>(0.5, 1, 1)));
    constexpr auto wall_z{10.0f};
    constexpr Point ray_origin{0, 0, -5};
    // Convert canvas pixels to world coordinates:
    // world_x: starts at -half (left edge) and increases with x
    // world_y: starts at +half (top edge) and decreases with y (canvas y is inverted)
    for (int y = 0; y < canvas.height; ++y) {
        const auto world_y{half - pixel_size * static_cast<float>(y)};
        for (int x = 0; x < canvas.width; ++x) {
            const auto world_x{-half + pixel_size * static_cast<float>(x)};
            Point position{.x = world_x, .y = world_y, .z = wall_z};
            Ray r{ray_origin, Vector::normalize(Vector(position - ray_origin))};
            auto xs{intersect(shape, r)};
            if (hit(xs).has_value()) {
                canvas.write_pixel(x, y, colour);
            }
        }
    }
    save_canvas(canvas, "sphere.ppm");
}
