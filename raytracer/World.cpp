//
// Created by chaku on 17/03/2026.
//

#include "MatrixImpl.hpp"
#include "World.hpp"
#include "Utils.hpp"

#include <algorithm>

namespace raytracer {
    World World::create_default_world() {
        World world;
        Geom &s1 = world.add(Sphere{});
        s1.material = {.colour = Colour{.r = 0.8, .g = 1.0, .b = 0.6}, .diffuse = 0.7, .specular = 0.2};
        Geom &s2 = world.add(Sphere{});
        s2.set_transform(scale<double>(0.5, 0.5, 0.5));
        world.light = {.position = Point{.x = -10, .y = 10, .z = -10}, .intensity = Colour{.r = 1, .g = 1, .b = 1}};
        return world;
    }

    Geom &World::add(AnyShape shape) {
        return *objects.emplace_back(std::make_unique<Geom>(std::move(shape)));
    }

    std::vector<Intersection> World::intersect_world(const World &world, const Ray &ray) {
        std::vector<Intersection> intersections;
        for (const auto &obj: world.objects) {
            // only root objects: a child is intersected through its parent group, never on its own
            if (obj->parent == nullptr) {
                intersections.append_range(intersect(*obj, ray));
            }
        }
        std::ranges::sort(intersections, [](const Intersection &a, const Intersection &b) {
            return (a.t < b.t);
        });
        return intersections;
    }

    Colour World::colour_at(const World &world, const Ray &ray, uint8_t remaining) {
        if (remaining == 0) {
            return colours::black;
        }
        const auto ray_world_intersections{intersect_world(world, ray)};
        if (const auto &maybe_hit{hit(ray_world_intersections)};
            maybe_hit.has_value()) {
            return shade_hit(world, prepare_computations(maybe_hit.value(), ray, ray_world_intersections), --remaining);
        }
        return Colour{.r = 0, .g = 0, .b = 0};
    }

    Ray World::ray_for_pixel(const Camera &camera, double px, double py) {
        const double x_offset{(px + 0.5) * camera.pixel_size};
        const double y_offset{(py + 0.5) * camera.pixel_size};

        const double world_x{camera.half_width - x_offset};
        const double world_y{camera.half_height - y_offset};

        const auto camera_transform_inverse{inverse(camera.transform)};
        if (!camera_transform_inverse.has_value()) {
            throw std::invalid_argument("Camera transform must be invertible");
        }
        const Point pixel{
            transform(Point{.x = static_cast<float>(world_x), .y = static_cast<float>(world_y), .z = -1.f},
                      camera_transform_inverse.value())
        };
        const Point origin{transform(Point{}, camera_transform_inverse.value())};

        const auto direction{Vector::normalize(pixel - origin)};
        return {.origin = origin, .direction = direction};
    }

    bool World::is_shadowed(const Point point) const {
        if (!light.has_value())
            return false;
        const auto point_to_light{light.value().position - point};
        const auto distance{Vector::magnitude(point_to_light)};
        const Ray point_to_light_ray{.origin = point, .direction = Vector::normalize(point_to_light)};
        const auto h{hit(intersect_world(*this, point_to_light_ray))};
        if (!h.has_value() || h.value().t >= distance) {
            return false;
        }
        return true;
    }

    Colour World::refracted_colour(const World &world, const Computations &comps, uint8_t remaining) {
        if (remaining == 0) {
            return colours::black;
        }
        const auto &mat = comps.object->material;
        if (mat.transparency == 0.0f) return colours::black;
        // Snell's law calculations
        // sin θᵢ / sin θₜ = n2 / n1
        // ratio of refractive index of first medium to refractive index of second medium
        const float n_ratio{comps.n1 / comps.n2};
        // cos θᵢ is the same as the dot product of the two vectors
        const float cos_i{Vector::dot(comps.eye_vector, comps.normal_vector)};
        const float sin2_t{utils::square(n_ratio) * (1 - utils::square(cos_i))};
        // check for Total internal reflection
        if (sin2_t > 1.0f) { return colours::black; }
        // cos θₜ via trigonometric identity
        const float cos_t{std::sqrt(1.0f - sin2_t)};
        // direction of refracted ray
        const Vector direction{comps.normal_vector * (n_ratio * cos_i - cos_t) - comps.eye_vector * n_ratio};
        // create refracted ray
        const Ray refracted_ray{.origin = comps.under_point, .direction = direction};
        // Find the color of the refracted ray, making sure to multiply by the transparency value to account for any opacity
        return colour_at(world, refracted_ray, --remaining) * mat.transparency;
    }

    Colour World::reflected_colour(const World &world, const Computations &comps, uint8_t remaining) {
        if (remaining == 0) {
            return colours::black;
        }
        const auto &mat = comps.object->material;
        if (mat.reflective == 0.0f) return colours::black;
        const Ray &reflect_ray{.origin = comps.over_point, .direction = comps.reflect_vector};
        const auto &colour = colour_at(world, reflect_ray, --remaining);
        return colour * mat.reflective;
    }

    float World::schlick(const Computations &comps) {
        // find the cosine of the angle between the eye and normal vectors
        float cos{Vector::dot(comps.eye_vector, comps.normal_vector)};
        if (comps.n1 > comps.n2) {
            const float n_ratio{comps.n1 / comps.n2};
            const float sin2_t{utils::square(n_ratio) * (1 - utils::square(cos))};
            if (sin2_t > 1.0f) {
                return 1.0f;
            }
            // compute cosine of theta_t using trig identity
            cos = std::sqrt(1.0f - sin2_t);
        }
        const float r0{utils::square((comps.n1 - comps.n2) / (comps.n1 + comps.n2))};
        return r0 + (1.0f - r0) * std::powf((1.0f - cos), 5);
    }
}
