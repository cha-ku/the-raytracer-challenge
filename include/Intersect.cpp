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

    Vector normal_at(const Sphere &s, const Point &world_point) {
        const auto &sphere_transform_inverse_expected{inverse(s.transform)};
        if (!sphere_transform_inverse_expected.has_value()) {
            return {};
        }
        const Container sphere_transform_inverse{sphere_transform_inverse_expected.value()};
        const Container object_point_container{multiply(sphere_transform_inverse, make_container(world_point))};
        const Point object_point{
            static_cast<float>(object_point_container.m_data[0]), static_cast<float>(object_point_container.m_data[1]),
            static_cast<float>(object_point_container.m_data[2])
        };
        const Vector object_normal{object_point - Point(0, 0, 0)};
        const Container world_normal_container = multiply(transpose(sphere_transform_inverse),
                                                          make_container(object_normal));
        const Vector world_normal{
            static_cast<float>(world_normal_container.m_data[0]),
            static_cast<float>(world_normal_container.m_data[1]),
            static_cast<float>(world_normal_container.m_data[2])
        };
        return Vector::normalize(world_normal);
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

    Vector Sphere::normal_at(const Point &point) {
        return {point - Point()};
    }

    void Sphere::set_transform(const Container<double> &t) {
        transform = t;
    }

    std::optional<Intersection> hit(const std::vector<Intersection> &intersections) {
        std::optional<Intersection> result;
        for (const auto &intersection: intersections) {
            if (intersection.t >= 0) {
                if (!result.has_value() || intersection.t < result->t) {
                    result = intersection;
                }
            }
        }
        return result;
    }
}
