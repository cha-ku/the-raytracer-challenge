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

    std::vector<Intersection> intersect(const Sphere &sphere, const Ray &ray);

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
