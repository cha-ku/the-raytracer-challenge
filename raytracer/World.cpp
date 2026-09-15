//
// Created by chaku on 17/03/2026.
//

#include "MatrixImpl.hpp"
#include "World.hpp"

#include <algorithm>

namespace raytracer {
    World World::create_default_world() {
        World world;
        Sphere s1 = Sphere::make_sphere();
        s1.material = {.colour = {0.8, 1.0, 0.6}, .diffuse = 0.7, .specular = 0.2};
        Sphere s2 = Sphere::make_sphere();
        s2.set_transform(scale<double>(0.5, 0.5, 0.5));
        world.objects = {s1, s2};
        world.light = {Point{-10, 10, -10}, Colour{1, 1, 1}};
        return world;
    }

    std::vector<Intersection> World::intersect_world(const World& world, const Ray& ray) {
        std::vector<Intersection> intersections;
        for (const auto& obj : world.objects) {
            intersections.append_range(intersect(obj, ray));
        }
        std::ranges::sort(intersections, [](const Intersection& a, const Intersection& b) {
            return (a.t < b.t);
        });
        return intersections;
    }

    Colour World::colour_at(const World &world, const Ray &ray) {
        const auto ray_world_intersections{intersect_world(world, ray)};
        if (const auto& maybe_hit{hit(ray_world_intersections)};
            maybe_hit.has_value()) {
            return shade_hit(world, prepare_computations(maybe_hit.value(), ray));
        }
        return Colour{0, 0, 0};
    }

    Ray World::ray_for_pixel(const Camera &camera, double px, double py) {
        const double x_offset{(px + 0.5) * camera.pixel_size};
        const double y_offset{(py + 0.5) * camera.pixel_size};

        const double world_x{camera.half_width - x_offset};
        const double world_y{camera.half_height - y_offset};

        const Matrix pixel_matrix{
            multiply(inverse(camera.transform).value(),
                make_matrix(Point{ static_cast<float>(world_x), static_cast<float>(world_y), -1.f}))
        };
        const Point pixel{
            static_cast<float>(pixel_matrix[0, 0]),
            static_cast<float>(pixel_matrix[1, 0]),
            static_cast<float>(pixel_matrix[2, 0])
        };

        const Matrix origin_matrix{multiply(inverse(camera.transform).value(), make_matrix(Point{}))};
        const Point origin{
            static_cast<float>(origin_matrix[0, 0]),
            static_cast<float>(origin_matrix[1, 0]),
            static_cast<float>(origin_matrix[2, 0])
        };

        const auto direction{Vector::normalize(pixel - origin)};
        return {origin, direction};
    }

    bool World::is_shadowed(const Point point) const {
        if (!light.has_value())
            return false;
        const auto point_to_light{light.value().position - point};
        const auto distance{Vector::magnitude(point_to_light)};
        const Ray point_to_light_ray{.origin = point , .direction = Vector::normalize(point_to_light)};
        const auto h{hit(intersect_world(*this, point_to_light_ray))};
        if (!h.has_value() || h.value().t >= distance) {
            return false;
        }
        return true;
    }
}
