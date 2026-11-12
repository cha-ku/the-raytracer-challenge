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

    void save_canvas(const raytracer::Canvas &canvas, const std::string &filename) {
        const auto &retval = raytracer::canvas_to_ppm(canvas, filename);
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
    constexpr Environment environment{
        .gravity = Vector{.x = 0, .y = -0.1, .z = 0}, .wind = Vector{.x = -0.01, .y = 0, .z = 0}
    };
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
    const Point centre{.x = static_cast<float>(canvas.width) / 2.f, .y = 0, .z = static_cast<float>(canvas.height) / 2};
    canvas.write_pixel(static_cast<uint32_t>(centre.x), static_cast<uint32_t>(centre.z),
                       Colour{.r = 1, .g = 0, .b = 0});
    const float radius{3 / 8.f * static_cast<float>(canvas.width)};
    // 12 o'clock position relative to origin
    const auto twelve_oclock = make_matrix({.x = 0, .y = 0, .z = -radius});

    for (int i = 0; i < 12; ++i) {
        const auto rad = static_cast<float>(i) * std::numbers::pi_v<float> / 6.f;
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
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error at hour " << i << ": " << e.what() << "\n";
        }
    }
    save_canvas(canvas, "clock.ppm");
}

void simulate_sphere() {
    using namespace raytracer;
    constexpr auto canvas_pixels{256u};
    constexpr auto wall_size{7.f};
    constexpr auto pixel_size{wall_size / canvas_pixels};
    constexpr auto half{wall_size / 2};
    Canvas canvas{canvas_pixels, canvas_pixels};
    constexpr Colour colour{.r = 1.f};
    Geom shape = make_sphere();
    // shrink it along the y axis
    shape.set_transform(scale<double>(1, 0.5, 1));
    // shrink it along the x axis
    shape.set_transform(scale<double>(0.5, 1, 1));
    // shrink it, and rotate it
    shape.set_transform(multiply(rotation_z(std::numbers::pi / 4), scale<double>(0.5, 1, 1)));
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
    constexpr auto pixel_size{wall_size / canvas_pixels};
    constexpr auto half{wall_size / 2};
    Canvas canvas{canvas_pixels, canvas_pixels};
    Geom sphere = make_sphere();
    sphere.material.colour = Colour{.r = 1, .g = 0.2, .b = 1};
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
                Vector normal = normal_at(*object, point);
                Vector eye = -r.direction;
                Colour pixel_colour = lighting(*object, object->material, point_light, point, eye, normal, false);
                canvas.write_pixel(x, y, pixel_colour);
            }
        }
    }
    save_canvas(canvas, "material_sphere.ppm");
}

void simulate_multiple_spheres() {
    using namespace raytracer;
    using namespace std::numbers;

    // owns every object in the scene; each world.add() returns the new object to configure
    World world;
    Geom &floor = world.add(Plane{});
    floor.material.colour = Colour{.r = 0.4f, .g = 0.6f, .b = 0.8f};
    floor.material.specular = 0.0f;
    CheckersPattern floor_pattern = CheckersPattern::create(colours::periwinkle, colours::vivid_blue);
    floor.material.pattern = floor_pattern;

    Geom &backdrop = world.add(Plane{});
    backdrop.set_transform(multiply(translation<double>(0, 0, 3.0), rotation_x(pi_v<double> / 2)));
    backdrop.material.colour = Colour{.r = 0.9f, .g = 0.85f, .b = 0.8f};
    backdrop.material.specular = 0.0f;

    Geom &middle = world.add(Sphere{});
    middle.set_transform(translation<double>(-0.5, 1, 0.5));
    middle.material.colour = Colour{.r = 0.1f, .g = 1.f, .b = 0.5f};
    middle.material.diffuse = 0.7f;
    middle.material.specular = 0.3f;

    Geom &right_sphere = world.add(Sphere{});
    right_sphere.set_transform(multiply(translation<double>(1.5, 0.5, -0.5), scale<double>(0.5, 0.5, 0.5)));
    right_sphere.material.colour = Colour{.r = 0.5f, .g = 1.f, .b = 0.1f};
    right_sphere.material.diffuse = 0.7f;
    right_sphere.material.specular = 0.3f;

    Geom &left_sphere = world.add(Sphere{});
    left_sphere.set_transform(multiply(translation<double>(-1.5, 0.33, -0.75), scale<double>(0.33, 0.33, 0.33)));
    left_sphere.material.colour = Colour{.r = 1.f, .g = 0.8f, .b = 0.1f};
    left_sphere.material.diffuse = 0.7f;
    left_sphere.material.specular = 0.3f;

    world.light = PointLight{
        .position = Point{.x = -10, .y = 10, .z = -10}, .intensity = Colour{.r = 1, .g = 1, .b = 1}
    };

    Camera camera{.hsize = 600, .vsize = 400, .field_of_view = pi_v<double> / 2};
    camera.transform = view_transform(Point{.x = 0, .y = 1.5f, .z = -5}, Point{.x = 0, .y = 1, .z = 0},
                                      Vector{.x = 0, .y = 1, .z = 0});

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

    // owns every object in the scene; each world.add() returns the new object to configure
    World world;

    // Lightly reflective checkered floor, so the spheres show up in it
    Geom &floor = world.add(Plane{});
    floor.material.specular = 0.0f;
    floor.material.reflective = 0.3f;
    floor.material.pattern = CheckersPattern::create(colours::periwinkle, colours::vivid_blue);

    // Non-reflective striped backdrop behind everything; stripes rotated to run vertically
    Geom &backdrop = world.add(Plane{});
    backdrop.set_transform(multiply(translation<double>(0, 0, 6.0), rotation_x(pi_v<double> / 2)));
    backdrop.material.specular = 0.0f;
    StripedPattern backdrop_pattern = StripedPattern::create(Colour{.r = 0.9f, .g = 0.85f, .b = 0.8f},
                                                             Colour{.r = 0.75f, .g = 0.7f, .b = 0.65f});
    backdrop_pattern.m_transform = scale<double>(0.5, 0.5, 0.5);
    backdrop.material.pattern = backdrop_pattern;

    // Large mirror sphere in the middle: dark base colour, almost fully reflective
    Geom &mirror = world.add(Sphere{});
    mirror.set_transform(translation<double>(0, 1, 0.5));
    mirror.material.colour = Colour{.r = 0.05f, .g = 0.05f, .b = 0.05f};
    mirror.material.diffuse = 0.2f;
    mirror.material.specular = 1.0f;
    mirror.material.shininess = 300.0f;
    mirror.material.reflective = 0.9f;

    // Gradient sphere on the left: one red → yellow fade across its width (x from -1 to 1 in object space)
    Geom &gradient_sphere = world.add(Sphere{});
    gradient_sphere.set_transform(multiply(translation<double>(-2.2, 0.7, -0.3), scale<double>(0.7, 0.7, 0.7)));
    GradientPattern gradient = GradientPattern::create(colours::red, colours::yellow);
    gradient.m_transform = multiply(translation<double>(-1, 0, 0), scale<double>(2, 1, 1));
    gradient_sphere.material.pattern = gradient;
    gradient_sphere.material.diffuse = 0.7f;
    gradient_sphere.material.specular = 0.3f;

    // Ring sphere on the right: tight rings tilted towards the camera, slightly reflective
    Geom &ring_sphere = world.add(Sphere{});
    ring_sphere.set_transform(multiply(translation<double>(2.1, 0.6, -0.4), scale<double>(0.6, 0.6, 0.6)));
    RingPattern rings = RingPattern::create(colours::white, colours::magenta);
    rings.m_transform = multiply(rotation_x(pi_v<double> / 2), scale<double>(0.2, 0.2, 0.2));
    ring_sphere.material.pattern = rings;
    ring_sphere.material.reflective = 0.2f;

    // Small striped sphere in front: thin diagonal stripes, slightly reflective
    Geom &striped_sphere = world.add(Sphere{});
    striped_sphere.set_transform(multiply(translation<double>(-0.9, 0.35, -1.8), scale<double>(0.35, 0.35, 0.35)));
    StripedPattern stripes = StripedPattern::create(colours::green, colours::white);
    stripes.m_transform = multiply(rotation_z(pi_v<double> / 4), scale<double>(0.25, 0.25, 0.25));
    striped_sphere.material.pattern = stripes;
    striped_sphere.material.reflective = 0.15f;

    // Small plain glossy sphere in front on the right, non-reflective for contrast
    Geom &plain_sphere = world.add(Sphere{});
    plain_sphere.set_transform(multiply(translation<double>(1.0, 0.3, -1.9), scale<double>(0.3, 0.3, 0.3)));
    plain_sphere.material.colour = colours::cyan;
    plain_sphere.material.diffuse = 0.7f;
    plain_sphere.material.specular = 0.5f;

    // Glass sphere front and centre: almost no surface colour of its own, mostly transparent and reflective,
    // with glass's refractive index, so the scene behind it shows through bent and flipped
    Geom &glass_sphere = world.add(Sphere{});
    glass_sphere.set_transform(multiply(translation<double>(0.1, 0.5, -3.0), scale<double>(0.5, 0.5, 0.5)));
    glass_sphere.material.colour = colours::black;
    glass_sphere.material.ambient = 0.0f;
    glass_sphere.material.diffuse = 0.1f;
    glass_sphere.material.specular = 1.0f;
    glass_sphere.material.shininess = 300.0f;
    glass_sphere.material.reflective = 0.9f;
    glass_sphere.material.transparency = 0.9f;
    glass_sphere.material.refractive_index = 1.5f;

    // Green glass cube front right, turned 45° about y so an edge faces the camera. Refracted light isn't
    // tinted by the material, so the green comes from the surface colour; it needs more diffuse than the
    // clear glass sphere to show, and correspondingly a bit less transparency
    Geom &glass_cube = world.add(Cube{});
    glass_cube.set_transform(multiply(translation<double>(1.9, 0.4, -2.6),
                                      multiply(rotation_y(pi_v<double> / 4), scale<double>(0.4, 0.4, 0.4))));
    glass_cube.material.colour = Colour{.r = 0.1f, .g = 0.6f, .b = 0.2f};
    glass_cube.material.ambient = 0.1f;
    glass_cube.material.diffuse = 0.3f;
    glass_cube.material.specular = 1.0f;
    glass_cube.material.shininess = 300.0f;
    glass_cube.material.reflective = 0.5f;
    glass_cube.material.transparency = 0.7f;
    glass_cube.material.refractive_index = 1.5f;

    // Closed gold cylinder front left: a short, slightly reflective metal post standing on the floor.
    // In object space it runs from y = 0 to y = 1 with radius 1; the scale makes it 1.2 tall and 0.35 wide
    Geom &gold_cylinder = world.add(Cylinder{.minimum = 0.0f, .maximum = 1.0f, .closed = true});
    gold_cylinder.set_transform(multiply(translation<double>(-2.0, 0.0, -2.4), scale<double>(0.35, 1.2, 0.35)));
    gold_cylinder.material.colour = Colour{.r = 0.8f, .g = 0.6f, .b = 0.2f};
    gold_cylinder.material.diffuse = 0.6f;
    gold_cylinder.material.specular = 0.9f;
    gold_cylinder.material.shininess = 200.0f;
    gold_cylinder.material.reflective = 0.3f;

    // Traffic cone front left, nearest the camera: only the lower half of the double cone (y from -1 to 0), so the
    // apex is at the top and the radius grows to 1 at the base. Scaled to 1.0 tall with a 0.4 base radius and lifted
    // by its height so the base sits on the floor. Placed left of the camera's line to the striped sphere so it
    // doesn't hide it. Left open: the base is flat on the floor and never seen, and local_normal_at for cones has
    // no cap normals
    Geom &traffic_cone = world.add(Cone{.minimum = -1.0f, .maximum = 0.0f});
    traffic_cone.set_transform(multiply(translation<double>(-1.0, 1.0, -4.2), scale<double>(0.4, 1.0, 0.4)));
    traffic_cone.material.diffuse = 0.8f;
    traffic_cone.material.specular = 0.3f;
    // Horizontal bands: stripes vary along pattern-space x, so rotating the pattern 90° about z makes them vary along
    // the cone's own y instead. Scaling by 0.2 gives 5 bands over the cone's height of 1 (y = -1 to 0). Patterns
    // are looked up in object space, so the cone's own scale doesn't stretch them. floor(5y) runs -5..-1 and
    // stripe_at picks colour a for even values, so the bands from the base up are orange, white, orange, white,
    // orange tip
    StripedPattern cone_bands = StripedPattern::create(colours::white, Colour{.r = 1.0f, .g = 0.4f, .b = 0.1f});
    cone_bands.m_transform = multiply(rotation_z(pi_v<double> / 2), scale<double>(0.2, 0.2, 0.2));
    traffic_cone.material.pattern = cone_bands;

    world.light = PointLight{.position = Point{.x = -10, .y = 10, .z = -10}, .intensity = colours::white};

    Camera camera{.hsize = 600, .vsize = 400, .field_of_view = pi_v<double> / 3};
    // pull back camera
    constexpr Point to{.x = 0, .y = 0.8f, .z = 0};
    constexpr Point from{to + Vector{.x = 0, .y = 1, .z = -6} * 1.4f}; // = (0, 2.2, -8.4)
    camera.transform = view_transform(from, to, Vector{.x = 0, .y = 1, .z = 0});

    Canvas canvas{camera.hsize, camera.vsize};
    for (uint32_t y = 0; y < camera.vsize; ++y) {
        for (uint32_t x = 0; x < camera.hsize; ++x) {
            const auto ray = World::ray_for_pixel(camera, x, y);
            canvas.write_pixel(x, y, World::colour_at(world, ray, max_depth));
        }
    }
    save_canvas(canvas, "reflections.ppm");
}

void simulate_hexagon() {
    using namespace raytracer;
    World world;
    Geom &hex = world.add(Group{});
    for (int i = 0; i < 6; ++i) {
        Geom &corner = world.add(Sphere{});
        /*
        set_transform(corner, translation(0, 0, -1) * scaling(0.25, 0.25, 0.25))
        */
        corner.set_transform(multiply(translation(0.0, 0.0, -1.0), scale(0.25, 0.25, 0.25)));

        Geom &edge = world.add(Cylinder{.minimum = 0, .maximum = 1.0f, .closed = true});
        /*
        set_transform(edge, translation(0, 0, -1) * rotation_y(-π/6) * rotation_z(-π/2) * scaling(0.25, 1, 0.25))
        */
        edge.set_transform(multiply(translation(0.0, 0.0, -1.0),
                             multiply(rotation_y(-std::numbers::pi_v<double> / 6),
                             multiply(rotation_z(-std::numbers::pi_v<double> / 2),
                             scale<double>(0.25, 1, 0.25)))));

        Geom &side = world.add(Group{});
        add_child(side, corner);
        add_child(side, edge);
        side.set_transform(rotation_y(i * std::numbers::pi_v<double> / 3));
        add_child(hex, side);
    }
    world.light = PointLight{.position = Point{.x = -10, .y = 10, .z = -10}, .intensity = colours::white};

    Camera camera{.hsize = 600, .vsize = 400, .field_of_view = std::numbers::pi_v<double> / 3};
    // pull back camera
    constexpr Point to{.x = 0, .y = 0.8f, .z = 0};
    constexpr Point from{to + Vector{.x = 0, .y = 1, .z = -6} * 1.4f}; // = (0, 2.2, -8.4)
    camera.transform = view_transform(from, to, Vector{.x = 0, .y = 1, .z = 0});

    constexpr uint8_t max_depth = 15;
    Canvas canvas{camera.hsize, camera.vsize};
    for (uint32_t y = 0; y < camera.vsize; ++y) {
        for (uint32_t x = 0; x < camera.hsize; ++x) {
            const auto ray = World::ray_for_pixel(camera, x, y);
            canvas.write_pixel(x, y, World::colour_at(world, ray, max_depth));
        }
    }
    save_canvas(canvas, "hexagon.ppm");
}
