//
// Created by chaku on 28/12/2025.
//

#ifndef THE_RAYTRACER_CHALLENGE_INTERSECT_HPP
#define THE_RAYTRACER_CHALLENGE_INTERSECT_HPP

#include "Point.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Material.hpp"
#include "Shape.hpp"

#include <concepts>
#include <vector>
#include <optional>

namespace raytracer {
    struct World;

    struct Ray {
        Point origin{};
        Vector direction{};
    };

    Point position(const Ray &ray, float distance);

    Ray transform(const Ray &ray, const Matrix<double> &matrix);

    Point transform(const Point &point, const Matrix<double> &matrix);

    // Shared by every concrete Shape: converts a world-space point into the
    // shape's object space, and a freshly-computed object-space normal back
    // into world space (by the inverse transpose of the transform), so
    // concrete shapes only need to implement their own object-space
    // intersect/normal math. Callers pass in the shape's already-inverted
    // transform (rather than the Shape itself) so a single inversion can be
    // shared across both a point/ray conversion and a normal conversion.
    Point to_object_space(const Point &point, const Matrix<double> &inverse_transform);

    Vector to_world_normal(const Vector &object_normal, const Matrix<double> &inverse_transform);

    struct Sphere : Shape, ShapeFactory<Sphere> {
        explicit Sphere(const uint32_t id) : Shape(id, ShapeType::Sphere) {}

        static Sphere make_sphere() { return make(); }
    };

    struct Intersection {
        Sphere object;
        float t{};
    };

    template<typename... Args>
    std::vector<Intersection> intersections(Args&&... args) {
        return std::vector<Intersection>{std::forward<Args>(args)...};
    }

    Vector normal_at(const Sphere& s, const Point& world_point);

    // Concrete, per-shape object-space intersection math. Takes the ray
    // already converted into the shape's own object space (see intersect
    // below). Adding a new shape type means adding its own overload here.
    std::vector<Intersection> local_intersect(const Sphere &sphere, const Ray &local_ray);

    // A shape type S is Intersectable if it has its own local_intersect
    // overload, found via ADL, taking S and a local-space Ray and returning
    // std::vector<Intersection>. This turns a missing/mismatched
    // local_intersect overload into a clear "constraints not satisfied"
    // error at the intersect() call site, instead of a confusing failure
    // deep inside intersect()'s template body.
    template<typename S>
    concept Intersectable = requires(const S &shape, const Ray &local_ray) {
        { local_intersect(shape, local_ray) } -> std::same_as<std::vector<Intersection>>;
    };

    // Shared by every concrete Shape: converts a world-space ray into the
    // shape's object space (by the inverse of its transform), then
    // dispatches to that shape's own local_intersect (resolved by overload
    // resolution on the concrete shape type S).
    template<Intersectable S>
    std::vector<Intersection> intersect(const S &shape, const Ray &ray) {
        const auto shape_transform_inverse{inverse(shape.m_transform)};
        if (!shape_transform_inverse.has_value()) {
            return {};
        }
        const Ray local_ray{transform(ray, shape_transform_inverse.value())};
        return local_intersect(shape, local_ray);
    }

    std::optional<Intersection> hit(const std::vector<Intersection> &intersections);


    struct Computations {
        float t{};
        Sphere object;
        Point point{};
        Vector eye_vector{};
        Vector normal_vector{};
        bool inside{};
        Point over_point{};
        explicit Computations(const Intersection& intersection, const Ray& ray);
    };

    Computations prepare_computations(const Intersection &intersection, const Ray &ray);

    Colour shade_hit(const World& world, const Computations& computations);

    Matrix<double> view_transform(const Point &from, const Point &to, const Vector &up);

}

#endif //THE_RAYTRACER_CHALLENGE_INTERSECT_HPP
