//
// Created by chaku on 28/12/2025.
//

#ifndef THE_RAYTRACER_CHALLENGE_INTERSECT_HPP
#define THE_RAYTRACER_CHALLENGE_INTERSECT_HPP

#include "Point.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "MatrixImpl.hpp"
#include "Material.hpp"
#include "Geom.hpp"
#include "Ray.hpp"

#include <concepts>
#include <variant>
#include <vector>
#include <optional>

namespace raytracer {
    struct World;

    struct Intersection {
        // Pointer to the scene object that was hit
        const Geom *object{nullptr};
        float t{};

        bool operator==(const Intersection &) const = default;
    };

    template<typename... Args>
    std::vector<Intersection> intersections(Args &&... args) {
        return std::vector<Intersection>{std::forward<Args>(args)...};
    }

    // Concrete, per-shape object-space normal math. Takes the point already converted into the shape's own
    // object space (see normal_at below). Adding a new shape type means adding its own overload here.
    Vector local_normal_at(const Sphere &, const Point &local_point);

    Vector local_normal_at(const Plane &, const Point &);

    Vector local_normal_at(const Cube &, const Point &local_point);

    Vector local_normal_at(const Cylinder &cylinder, const Point &local_point);

    Vector local_normal_at(const Cone &, const Point &local_point);

    [[noreturn]] Vector local_normal_at(const Group&, const Point&);

    Vector local_normal_at(const Triangle& triangle, const Point&);

    Vector local_normal_at(const TestShape &, const Point &local_point);

    // A shape type S is Normalable if it has its own local_normal_at overload taking S and a local-space Point
    // and returning a Vector.
    template<typename S>
    concept Normalable = requires(const S &shape, const Point &local_point)
    {
        { local_normal_at(shape, local_point) } -> std::same_as<Vector>;
    };

    // World-space normal of `geom` at a world-space point: converts the point into object space (through any
    // parents), dispatches to the shape's own local_normal_at, then converts the normal back out to world space.
    Vector normal_at(const Geom &geom, const Point &world_point);

    // Concrete, per-shape object-space intersection math. Takes the ray already converted into the shape's own
    // object space (see intersect below) and returns the t values where it hits. It knows nothing about which
    // Geom it belongs to; intersect() pairs each t with the Geom.
    std::vector<float> local_intersect(const Sphere &, const Ray &local_ray);

    std::vector<float> local_intersect(const Plane &, const Ray &local_ray);

    std::vector<float> local_intersect(const Cube &, const Ray &local_ray);

    std::vector<float> local_intersect(const Cylinder &cylinder, const Ray &local_ray);

    std::vector<float> local_intersect(const Cone &cone, const Ray &local_ray);

    std::vector<float> local_intersect(const Group &group, const Ray &local_ray);

    std::vector<float> local_intersect(const Triangle &triangle, const Ray &local_ray);

    std::vector<float> local_intersect(const TestShape &shape, const Ray &local_ray);

    // A shape type S is Intersectable if it has its own local_intersect overload taking S and a local-space Ray
    // and returning its t values.
    template<typename S>
    concept Intersectable = requires(const S &shape, const Ray &local_ray)
    {
        { local_intersect(shape, local_ray) } -> std::same_as<std::vector<float> >;
    };

    // Converts a world-space ray into `geom`'s object space (by its cached inverse transform), dispatches to the
    // shape's own local_intersect, and pairs every t value with `geom`.
    std::vector<Intersection> intersect(const Geom &geom, const Ray &ray);

    std::optional<Intersection> hit(const std::vector<Intersection> &intersections);


    struct Computations {
        float t{};
        const Geom *object{nullptr};
        Point point{};
        Vector eye_vector{};
        Vector normal_vector{};
        bool inside{};
        Point over_point{};
        Point under_point{};
        Vector reflect_vector{};
        float n1{1.0f};
        float n2{1.0f};

        explicit Computations(const Intersection &intersection, const Ray &ray);
    };

    Computations prepare_computations(const Intersection &intersection, const Ray &ray,
                                      const std::vector<Intersection>& intersections);

    Colour shade_hit(const World &world, const Computations &computations, uint8_t remaining);

    Matrix<double> view_transform(const Point &from, const Point &to, const Vector &up);
}

#endif //THE_RAYTRACER_CHALLENGE_INTERSECT_HPP
