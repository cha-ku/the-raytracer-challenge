//
// Created by chaku on 28/12/2025.
//

#include "Intersect.hpp"
#include "MatrixImpl.hpp"

namespace raytracer {

    Point position(const Ray &ray, const float distance) {
        return ray.origin + ray.direction * distance;
    }

    Ray transform(const Ray &ray, const Container<double> &matrix) {
        const Container<double> origin_container{make_container(Point{ray.origin.x, ray.origin.y, ray.origin.z})};
        const Container<double> direction_container{4, 1, std::vector{
            static_cast<double>(ray.direction.x),
            static_cast<double>(ray.direction.y),
            static_cast<double>(ray.direction.z),
            0.0
        }};

        const Container<double> new_origin{multiply(matrix, origin_container)};
        const Container<double> new_direction{multiply(matrix, direction_container)};

        return Ray{
            Point{static_cast<float>(new_origin.m_data[0]),
                  static_cast<float>(new_origin.m_data[1]),
                  static_cast<float>(new_origin.m_data[2])},
            Vector{static_cast<float>(new_direction.m_data[0]),
                   static_cast<float>(new_direction.m_data[1]),
                   static_cast<float>(new_direction.m_data[2])}
        };
    }

    std::vector<Intersection> intersect(const Sphere &sphere, const Ray &ray) {
        const auto inv = inverse(sphere.transform);
        if (!inv.has_value()) {
            return {};
        }
        const auto [origin, direction] = transform(ray, inv.value());

        const Vector sphere_to_ray{origin - Point(0, 0, 0)};
        const float a{Vector::dot(direction, direction)};
        const float b{2 * Vector::dot(direction, sphere_to_ray)};
        const float c{Vector::dot(sphere_to_ray, sphere_to_ray) - 1};
        const float discriminant{b * b - (4 * a * c)};
        if (discriminant < 0) {
            return {};
        }
        const float t1{(-b - std::sqrt(discriminant)) / (2 * a)};
        const float t2{(-b + std::sqrt(discriminant)) / (2 * a)};
        return {{sphere, t1}, {sphere, t2}};
    }

    Sphere Sphere::make_sphere() {
        return Sphere(++sphere_id);
    }

    void set_transform(Sphere& s, const Container<double> &t) {
        s.transform = t;
    }

    std::optional<Intersection> hit(const std::vector<Intersection> &intersections) {
        std::optional<Intersection> result;
        for (const auto &intersection : intersections) {
            if (intersection.t >= 0) {
                if (!result.has_value() || intersection.t < result->t) {
                    result = intersection;
                }
            }
        }
        return result;
    }
}