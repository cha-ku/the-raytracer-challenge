//
// Created by chaku on 28/12/2025.
//

#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "World.hpp"

namespace raytracer {
    Point position(const Ray &ray, const float distance) {
        return ray.origin + ray.direction * distance;
    }

    Ray transform(const Ray &ray, const Matrix<double> &matrix) {
        const Matrix<double> origin_matrix{make_matrix(Point{ray.origin.x, ray.origin.y, ray.origin.z})};
        const Matrix<double> direction_matrix{4, 1, std::vector{
            static_cast<double>(ray.direction.x),
            static_cast<double>(ray.direction.y),
            static_cast<double>(ray.direction.z),
            0.0
        }};

        const Matrix<double> new_origin{multiply(matrix, origin_matrix)};
        const Matrix<double> new_direction{multiply(matrix, direction_matrix)};

        return Ray{
            Point{static_cast<float>(new_origin[0, 0]),
                  static_cast<float>(new_origin[1, 0]),
                  static_cast<float>(new_origin[2, 0])},
            Vector{static_cast<float>(new_direction[0, 0]),
                   static_cast<float>(new_direction[1, 0]),
                   static_cast<float>(new_direction[2, 0])}
        };
    }

    Point transform(const Point &point, const Matrix<double> &matrix) {
        const Matrix point_matrix{multiply(matrix, make_matrix(point))};
        return Point{
            static_cast<float>(point_matrix[0, 0]),
            static_cast<float>(point_matrix[1, 0]),
            static_cast<float>(point_matrix[2, 0])
        };
    }

    Point to_object_space(const Point &point, const Matrix<double> &inverse_transform) {
        return transform(point, inverse_transform);
    }

    Vector to_world_normal(const Vector &object_normal, const Matrix<double> &inverse_transform) {
        const Matrix world_normal_matrix{
            multiply(transpose(inverse_transform), make_matrix(object_normal))
        };
        const Vector world_normal{
            static_cast<float>(world_normal_matrix[0, 0]),
            static_cast<float>(world_normal_matrix[1, 0]),
            static_cast<float>(world_normal_matrix[2, 0])
        };
        return Vector::normalize(world_normal);
    }

    Vector local_normal_at(const Sphere &sphere, const Point &local_point) {
        return local_point - Point(0, 0, 0);
    }

    Vector local_normal_at(const Plane &plane, const Point &local_point) {
        return Vector(0, 1, 0);
    }

    std::vector<Intersection> local_intersect(const Sphere &sphere, const Ray &local_ray) {
        const auto &[origin, direction] = local_ray;

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

    std::vector<Intersection> local_intersect(const Plane &plane, const Ray &local_ray) {
        if (std::abs(local_ray.direction.y) < utils::EPSILON) {
            return {};
        }
        const float t{-local_ray.origin.y / local_ray.direction.y};
        return {{plane, t}};
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

    Computations::Computations(const Intersection &intersection, const Ray &ray) : t{intersection.t},
        object{intersection.object}, point{position(ray, intersection.t)}, eye_vector{-ray.direction},
        normal_vector{std::visit([this](const auto &shape) { return normal_at(shape, point); }, object)} {
        if (Vector::dot(normal_vector, eye_vector) < 0) {
            inside = true;
            normal_vector = -normal_vector;
        }
        else {
            inside = false;
        }
        over_point = point + normal_vector * utils::EPSILON;
    }

    Computations prepare_computations(const Intersection &intersection, const Ray &ray) {
        return Computations(intersection, ray);
    }

    Colour shade_hit(const World &world, const Computations &computations) {
        return lighting(material_of(computations.object), world.light.value(), computations.point, computations.eye_vector,
                        computations.normal_vector, world.is_shadowed(computations.over_point));
    }

    // Coordinate system (right-handed):
    //
    //        +Y
    //         |
    //         |
    //         +------+X
    //        /
    //       /
    //     +Z (toward viewer)
    //
    // Camera looks down -Z by default.
    // view_transform(from=(0,0,0), to=(0,0,-1), up=(0,1,0)) == identity
    Matrix<double> view_transform(const Point &from, const Point &to, const Vector &up) {
        const Vector forward{Vector::normalize(to - from)};
        const Vector left{Vector::cross(forward, Vector::normalize(up))};
        const Vector true_up{Vector::cross(left, forward)};

        return multiply(Matrix<double>{
                            4, 4, std::array<double, 16>{
                                left.x, left.y, left.z, 0,
                                true_up.x, true_up.y, true_up.z, 0,
                                -forward.x, -forward.y, -forward.z, 0,
                                0, 0, 0, 1
                            }
                        }, translation<double>(-from.x, -from.y, -from.z));
    }
}
