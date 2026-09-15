//
// Created by chaku on 14/06/25.
//

#include <iostream>
#include "Point.hpp"
#include "Vector.hpp"
#include "Simulation.hpp"
#include "Canvas.hpp"
#include <numbers>

#include "Intersect.hpp"
#include "World.hpp"
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
    const auto twelve_oclock = make_matrix({0, 0, -radius});

    for (int i = 0; i < 12; ++i) {
        const auto rad = static_cast<float>(i) * std::numbers::pi_v<float>/6.f;
        const auto hour{rotation_y(rad)};
        try {
            // Rotate the point around origin
            const auto rotated = multiply(hour, twelve_oclock);
            // Translate to canvas center
            const auto x = rotated[0, 0] + centre.x;
            const auto z = rotated[2, 0] + centre.z;

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
    shape.set_transform(scale<double>(1, 0.5, 1));
    // shrink it along the x axis
    shape.set_transform(scale<double>(0.5, 1, 1));
    // shrink it, and rotate it
    shape.set_transform(multiply(rotation_z(std::numbers::pi/4),  scale<double>(0.5, 1, 1)));
    // shrink it, and skew it
    shape.set_transform(multiply(shearing(1, 0, 0, 0, 0, 0), scale<double>(0.5, 1, 1)));
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

void simulate_material_sphere() {
    using namespace raytracer;
    constexpr auto canvas_pixels{256u};
    constexpr auto wall_size{7.f};
    constexpr auto pixel_size{wall_size/canvas_pixels};
    constexpr auto half{wall_size / 2};
    Canvas canvas{canvas_pixels, canvas_pixels};
    Sphere sphere = Sphere::make_sphere();
    sphere.material.colour = Colour(1, 0.2, 1);
    constexpr auto wall_z{10.0f};

    // camera(eye) is the origin of our rays
    constexpr Point camera{0, 0, -5};
    constexpr PointLight point_light{{-10, 10, -10}, {1, 1, 1}};
    // Convert canvas pixels to world coordinates:
    // world_x: starts at -half (left edge) and increases with x
    // world_y: starts at +half (top edge) and decreases with y (canvas y is inverted)
    for (int y = 0; y < canvas.height; ++y) {
        const auto world_y{half - pixel_size * static_cast<float>(y)};
        for (int x = 0; x < canvas.width; ++x) {
            const auto world_x{-half + pixel_size * static_cast<float>(x)};
            Point p{.x = world_x, .y = world_y, .z = wall_z};
            Ray r{camera, Vector::normalize(Vector(p - camera))};
            auto xs{intersect(sphere, r)};
            if (hit(xs).has_value()) {
                auto [object, t] = hit(xs).value();
                Point point = position(r, t);
                Vector normal = normal_at(object, point);
                Vector eye = -r.direction;
                Colour pixel_colour = lighting(object.material, point_light, point, eye, normal, false);
                canvas.write_pixel(x, y, pixel_colour);
            }
        }
    }
    save_canvas(canvas, "material_sphere.ppm");
}

void simulate_multiple_spheres() {
    using namespace raytracer;
    using namespace std::numbers;

    Sphere floor{Sphere::make_sphere()};
    floor.transform = scale<double>(10, 0.01, 10);
    floor.material.colour = Colour{1.f, 0.9f, 0.9f};
    floor.material.specular = 0.0f;

    Sphere left_wall{Sphere::make_sphere()};
    left_wall.transform = multiply(
        multiply(multiply(translation<double>(0, 0, 5), rotation_y(-pi_v<double>/4)),
                 rotation_x(pi_v<double>/2)),
        scale<double>(10, 0.01, 10));
    left_wall.material = floor.material;

    Sphere right_wall{Sphere::make_sphere()};
    right_wall.transform = multiply(
        multiply(multiply(translation<double>(0, 0, 5), rotation_y(pi_v<double>/4)),
                 rotation_x(pi_v<double>/2)),
        scale<double>(10, 0.01, 10));
    right_wall.material = floor.material;

    Sphere middle{Sphere::make_sphere()};
    middle.transform = translation<double>(-0.5, 1, 0.5);
    middle.material.colour = Colour{0.1f, 1.f, 0.5f};
    middle.material.diffuse = 0.7f;
    middle.material.specular = 0.3f;

    Sphere right_sphere{Sphere::make_sphere()};
    right_sphere.transform = multiply(translation<double>(1.5, 0.5, -0.5), scale<double>(0.5, 0.5, 0.5));
    right_sphere.material.colour = Colour{0.5f, 1.f, 0.1f};
    right_sphere.material.diffuse = 0.7f;
    right_sphere.material.specular = 0.3f;

    Sphere left_sphere{Sphere::make_sphere()};
    left_sphere.transform = multiply(translation<double>(-1.5, 0.33, -0.75), scale<double>(0.33, 0.33, 0.33));
    left_sphere.material.colour = Colour{1.f, 0.8f, 0.1f};
    left_sphere.material.diffuse = 0.7f;
    left_sphere.material.specular = 0.3f;

    World world;
    world.objects = {floor, left_wall, right_wall, middle, right_sphere, left_sphere};
    world.light = PointLight{Point{-10, 10, -10}, Colour{1, 1, 1}};

    Camera camera{200, 100, pi_v<double>/3};
    camera.transform = view_transform(Point{0, 1.5f, -5}, Point{0, 1, 0}, Vector{0, 1, 0});

    Canvas canvas{camera.hsize, camera.vsize};
    for (uint32_t y = 0; y < camera.vsize; ++y) {
        for (uint32_t x = 0; x < camera.hsize; ++x) {
            const auto ray = World::ray_for_pixel(camera, x, y);
            canvas.write_pixel(x, y, World::colour_at(world, ray));
        }
    }
    save_canvas(canvas, "multiple_spheres.ppm");
}