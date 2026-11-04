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

namespace {
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
        return {.position = new_position, .velocity = new_velocity};
    }

    void save_canvas(const raytracer::Canvas& canvas, const std::string& filename) {
        const auto& retval = raytracer::canvas_to_ppm(canvas, filename);
        if (retval.has_value()) {
            std::cout << "Data written to " << filename << "\n";
        } else if (retval.error() == raytracer::CanvasError::invalid_path) {
            std::cout << "Cannot write file " << filename << " - invalid path\n";
        }
    }
}


void simulate_projectile() {
    using namespace raytracer;
    // Initialize projectile and environment
    constexpr Point start{.x = 0, .y = 1, .z = 0};
    const Vector velocity{Vector::normalize({.x = 1, .y = 1.8, .z = 0}) * 11.25};
    Projectile projectile{.position = start, .velocity = velocity};
    constexpr Environment environment{.gravity = Vector{.x = 0, .y = -0.1, .z = 0}, .wind = Vector{.x = -0.01, .y = 0, .z = 0}};
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
    const Point centre{.x = static_cast<float>(canvas.width)/2.f, .y = 0, .z = static_cast<float>(canvas.height)/2};
    canvas.write_pixel(static_cast<uint32_t>(centre.x), static_cast<uint32_t>(centre.z),  Colour{.r = 1, .g = 0, .b = 0});
    const float radius{3/8.f * static_cast<float>(canvas.width)};
    // 12 o'clock position relative to origin
    const auto twelve_oclock = make_matrix({.x = 0, .y = 0, .z = -radius});

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
                    Colour{.r = 1, .g = 1, .b = 1});
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
    constexpr Point ray_origin{.x = 0, .y = 0, .z = -5};
    // Convert canvas pixels to world coordinates:
    // world_x: starts at -half (left edge) and increases with x
    // world_y: starts at +half (top edge) and decreases with y (canvas y is inverted)
    for (int y = 0; y < canvas.height; ++y) {
        const auto world_y{half - pixel_size * static_cast<float>(y)};
        for (int x = 0; x < canvas.width; ++x) {
            const auto world_x{-half + pixel_size * static_cast<float>(x)};
            Point position{.x = world_x, .y = world_y, .z = wall_z};
            Ray r{.origin = ray_origin, .direction = Vector::normalize(Vector(position - ray_origin))};
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
    sphere.m_material.colour = Colour{.r = 1, .g = 0.2, .b = 1};
    constexpr auto wall_z{10.0f};

    // camera(eye) is the origin of our rays
    constexpr Point camera{.x = 0, .y = 0, .z = -5};
    constexpr PointLight point_light{.position = {.x = -10, .y = 10, .z = -10}, .intensity = {.r = 1, .g = 1, .b = 1}};
    // Convert canvas pixels to world coordinates:
    // world_x: starts at -half (left edge) and increases with x
    // world_y: starts at +half (top edge) and decreases with y (canvas y is inverted)
    for (int y = 0; y < canvas.height; ++y) {
        const auto world_y{half - pixel_size * static_cast<float>(y)};
        for (int x = 0; x < canvas.width; ++x) {
            const auto world_x{-half + pixel_size * static_cast<float>(x)};
            Point p{.x = world_x, .y = world_y, .z = wall_z};
            Ray r{.origin = camera, .direction = Vector::normalize(Vector(p - camera))};
            auto xs{intersect(sphere, r)};
            if (hit(xs).has_value()) {
                auto [object, t] = hit(xs).value();
                Point point = position(r, t);
                Vector normal = std::visit([&](const auto &shape) { return normal_at(shape, point); }, object);
                Vector eye = -r.direction;
                Colour pixel_colour = lighting(as_shape(object), material_of(object), point_light, point, eye, normal, false);
                canvas.write_pixel(x, y, pixel_colour);
            }
        }
    }
    save_canvas(canvas, "material_sphere.ppm");
}

void simulate_multiple_spheres() {
    using namespace raytracer;
    using namespace std::numbers;

    Plane floor{Plane::make_plane()};
    floor.m_material.colour = Colour{.r = 0.4f, .g = 0.6f, .b = 0.8f};
    floor.m_material.specular = 0.0f;
    CheckersPattern floor_pattern = CheckersPattern::create(colours::periwinkle, colours::vivid_blue);
    floor.m_material.pattern = floor_pattern;

    Plane backdrop{Plane::make_plane()};
    backdrop.set_transform(multiply(translation<double>(0, 0, 3.0), rotation_x(pi_v<double>/2)));
    backdrop.m_material.colour = Colour{.r = 0.9f, .g = 0.85f, .b = 0.8f};
    backdrop.m_material.specular = 0.0f;

    Sphere middle{Sphere::make_sphere()};
    middle.m_transform = translation<double>(-0.5, 1, 0.5);
    middle.m_material.colour = Colour{.r = 0.1f, .g = 1.f, .b = 0.5f};
    middle.m_material.diffuse = 0.7f;
    middle.m_material.specular = 0.3f;

    Sphere right_sphere{Sphere::make_sphere()};
    right_sphere.m_transform = multiply(translation<double>(1.5, 0.5, -0.5), scale<double>(0.5, 0.5, 0.5));
    right_sphere.m_material.colour = Colour{.r = 0.5f, .g = 1.f, .b = 0.1f};
    right_sphere.m_material.diffuse = 0.7f;
    right_sphere.m_material.specular = 0.3f;

    Sphere left_sphere{Sphere::make_sphere()};
    left_sphere.m_transform = multiply(translation<double>(-1.5, 0.33, -0.75), scale<double>(0.33, 0.33, 0.33));
    left_sphere.m_material.colour = Colour{.r = 1.f, .g = 0.8f, .b = 0.1f};
    left_sphere.m_material.diffuse = 0.7f;
    left_sphere.m_material.specular = 0.3f;

    World world;
    world.objects = {floor, backdrop, middle, right_sphere, left_sphere};
    world.light = PointLight{.position = Point{.x = -10, .y = 10, .z = -10}, .intensity = Colour{.r = 1, .g = 1, .b = 1}};

    Camera camera{.hsize = 600, .vsize = 400, .field_of_view = pi_v<double>/2};
    camera.transform = view_transform(Point{.x = 0, .y = 1.5f, .z = -5}, Point{.x = 0, .y = 1, .z = 0}, Vector{.x = 0, .y = 1, .z = 0});

    Canvas canvas{camera.hsize, camera.vsize};
    for (uint32_t y = 0; y < camera.vsize; ++y) {
        for (uint32_t x = 0; x < camera.hsize; ++x) {
            const auto ray = World::ray_for_pixel(camera, x, y);
            canvas.write_pixel(x, y, World::colour_at(world, ray, 255));
        }
    }
    save_canvas(canvas, "multiple_spheres.ppm");
}
void simulate_reflections() {
    using namespace raytracer;
    using namespace std::numbers;

    // colour_at, shade_hit and reflected_colour each decrement this, so every bounce costs 3: ~5 bounces
    constexpr uint8_t max_depth = 15;

    // Lightly reflective checkered floor, so the spheres show up in it
    Plane floor{Plane::make_plane()};
    floor.m_material.specular = 0.0f;
    floor.m_material.reflective = 0.3f;
    floor.m_material.pattern = CheckersPattern::create(colours::periwinkle, colours::vivid_blue);

    // Non-reflective striped backdrop behind everything; stripes rotated to run vertically
    Plane backdrop{Plane::make_plane()};
    backdrop.set_transform(multiply(translation<double>(0, 0, 6.0), rotation_x(pi_v<double> / 2)));
    backdrop.m_material.specular = 0.0f;
    StripedPattern backdrop_pattern = StripedPattern::create(Colour{.r = 0.9f, .g = 0.85f, .b = 0.8f},
                                                             Colour{.r = 0.75f, .g = 0.7f, .b = 0.65f});
    backdrop_pattern.m_transform = scale<double>(0.5, 0.5, 0.5);
    backdrop.m_material.pattern = backdrop_pattern;

    // Large mirror sphere in the middle: dark base colour, almost fully reflective
    Sphere mirror{Sphere::make_sphere()};
    mirror.set_transform(translation<double>(0, 1, 0.5));
    mirror.m_material.colour = Colour{.r = 0.05f, .g = 0.05f, .b = 0.05f};
    mirror.m_material.diffuse = 0.2f;
    mirror.m_material.specular = 1.0f;
    mirror.m_material.shininess = 300.0f;
    mirror.m_material.reflective = 0.9f;

    // Gradient sphere on the left: one red → yellow fade across its width (x from -1 to 1 in object space)
    Sphere gradient_sphere{Sphere::make_sphere()};
    gradient_sphere.set_transform(multiply(translation<double>(-2.2, 0.7, -0.3), scale<double>(0.7, 0.7, 0.7)));
    GradientPattern gradient = GradientPattern::create(colours::red, colours::yellow);
    gradient.m_transform = multiply(translation<double>(-1, 0, 0), scale<double>(2, 1, 1));
    gradient_sphere.m_material.pattern = gradient;
    gradient_sphere.m_material.diffuse = 0.7f;
    gradient_sphere.m_material.specular = 0.3f;

    // Ring sphere on the right: tight rings tilted towards the camera, slightly reflective
    Sphere ring_sphere{Sphere::make_sphere()};
    ring_sphere.set_transform(multiply(translation<double>(2.1, 0.6, -0.4), scale<double>(0.6, 0.6, 0.6)));
    RingPattern rings = RingPattern::create(colours::white, colours::magenta);
    rings.m_transform = multiply(rotation_x(pi_v<double> / 2), scale<double>(0.2, 0.2, 0.2));
    ring_sphere.m_material.pattern = rings;
    ring_sphere.m_material.reflective = 0.2f;

    // Small striped sphere in front: thin diagonal stripes, slightly reflective
    Sphere striped_sphere{Sphere::make_sphere()};
    striped_sphere.set_transform(multiply(translation<double>(-0.9, 0.35, -1.8), scale<double>(0.35, 0.35, 0.35)));
    StripedPattern stripes = StripedPattern::create(colours::green, colours::white);
    stripes.m_transform = multiply(rotation_z(pi_v<double> / 4), scale<double>(0.25, 0.25, 0.25));
    striped_sphere.m_material.pattern = stripes;
    striped_sphere.m_material.reflective = 0.15f;

    // Small plain glossy sphere in front on the right, non-reflective for contrast
    Sphere plain_sphere{Sphere::make_sphere()};
    plain_sphere.set_transform(multiply(translation<double>(1.0, 0.3, -1.9), scale<double>(0.3, 0.3, 0.3)));
    plain_sphere.m_material.colour = colours::cyan;
    plain_sphere.m_material.diffuse = 0.7f;
    plain_sphere.m_material.specular = 0.5f;

    World world;
    world.objects = {floor, backdrop, mirror, gradient_sphere, ring_sphere, striped_sphere, plain_sphere};
    world.light = PointLight{.position = Point{.x = -10, .y = 10, .z = -10}, .intensity = colours::white};

    Camera camera{.hsize = 600, .vsize = 400, .field_of_view = pi_v<double> / 3};
    camera.transform = view_transform(Point{.x = 0, .y = 1.8f, .z = -6}, Point{.x = 0, .y = 0.8f, .z = 0},
                                      Vector{.x = 0, .y = 1, .z = 0});

    Canvas canvas{camera.hsize, camera.vsize};
    for (uint32_t y = 0; y < camera.vsize; ++y) {
        for (uint32_t x = 0; x < camera.hsize; ++x) {
            const auto ray = World::ray_for_pixel(camera, x, y);
            canvas.write_pixel(x, y, World::colour_at(world, ray, max_depth));
        }
    }
    save_canvas(canvas, "reflections.ppm");
}
