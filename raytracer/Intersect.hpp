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
#include <vector>
#include <optional>
#include <limits>

namespace raytracer {
    struct World;

    struct Intersection {
        // Pointer to the scene object that was hit
        const Geom *object{nullptr};
        float t{};
        std::optional<float> u{};
        std::optional<float> v{};

        bool operator==(const Intersection &) const = default;
    };

    template<typename... Args>
    std::vector<Intersection> intersections(Args &&... args) {
        return std::vector<Intersection>{std::forward<Args>(args)...};
    }

    struct TriangleHit {
        float t{};
        float u{};
        float v{};
    };

    // Concrete, per-shape object-space normal math. Takes the point already converted into the shape's own
    // object space (see normal_at below). Adding a new shape type means adding its own overload here.
    Vector local_normal_at(const Sphere &, const Point &local_point);

    Vector local_normal_at(const Plane &, const Point &);

    Vector local_normal_at(const Cube &, const Point &local_point);

    Vector local_normal_at(const Cylinder &cylinder, const Point &local_point);

    Vector local_normal_at(const Cone &, const Point &local_point);

    [[noreturn]] Vector local_normal_at(const Group &, const Point &);

    Vector local_normal_at(const Triangle &triangle, const Point &);

    // Blends the corner normals by the hit's u/v (only a smooth triangle's own hits have them)
    Vector local_normal_at(const SmoothTriangle &smooth_triangle, const Point &, const Intersection &hit);

    [[noreturn]] Vector local_normal_at(const Csg &, const Point &);

    Vector local_normal_at(const TestShape &, const Point &local_point);

    // Template that lifts 2-argument functions to the 3 argument function as specified by the Normalable concept
    template<typename S>
        requires requires(const S &shape, const Point &p) { { local_normal_at(shape, p) } -> std::same_as<Vector>; }
    Vector local_normal_at(const S &shape, const Point &local_point, const Intersection &) {
        return local_normal_at(shape, local_point);
    }

    // A shape type S is Normalable if it has its own local_normal_at overload taking S and a local-space Point
    // and returning a Vector.
    template<typename S>
    concept Normalable = requires(const S &shape, const Point &local_point, const Intersection &hit)
    {
        { local_normal_at(shape, local_point, hit) } -> std::same_as<Vector>;
    };

    // World-space normal of `geom` at a world-space point: converts the point into object space (through any
    // parents), dispatches to the shape's own local_normal_at, then converts the normal back out to world space.
    // `hit` is the intersection the point came from; only shapes that interpolate their normal (smooth triangles) read it.
    Vector normal_at(const Geom &geom, const Point &world_point, const Intersection &hit = {});

    // Concrete, per-shape object-space intersection math. Takes the ray already converted into the shape's own
    // object space (see intersect below) and returns the t values where it hits. It knows nothing about which
    // Geom it belongs to; intersect() pairs each t with the Geom.
    std::vector<float> local_intersect(const Sphere &, const Ray &local_ray);

    std::vector<float> local_intersect(const Plane &, const Ray &local_ray);

    std::vector<float> local_intersect(const Cube &, const Ray &local_ray);

    std::vector<float> local_intersect(const Cylinder &cylinder, const Ray &local_ray);

    std::vector<float> local_intersect(const Cone &cone, const Ray &local_ray);

    std::vector<Intersection> local_intersect(const Group &group, const Ray &local_ray);

    std::vector<float> local_intersect(const Triangle &triangle, const Ray &local_ray);

    std::optional<TriangleHit> local_intersect(const SmoothTriangle &smooth_triangle, const Ray &local_ray);

    std::vector<Intersection> local_intersect(const Csg &csg, const Ray &local_ray);

    std::vector<float> local_intersect(const TestShape &shape, const Ray &local_ray);

    inline std::vector<Intersection> to_intersections(const Geom &geom, const std::vector<float> &ts) {
        std::vector<Intersection> result;
        result.reserve(ts.size());
        for (const auto t: ts) {
            result.push_back({.object = &geom, .t = t});
        }
        return result;
    }

    inline std::vector<Intersection> to_intersections(const Geom &geom, const std::optional<TriangleHit> &hit) {
        if (!hit) {
            return {};
        }
        return {Intersection{.object = &geom, .t = hit->t, .u = hit->u, .v = hit->v}};
    }

    inline std::vector<Intersection> to_intersections(const Geom &, std::vector<Intersection> intersections) {
        return intersections;
    }

    // A shape type S is Intersectable if it has its own local_intersect overload taking S and a local-space Ray
    // and returning its t values, or it returns an optional TriangleHit. This is encapsulated by the to_intersections method
    // which has an overload for both types.
    template<typename S>
    concept Intersectable = requires(const S &shape, const Ray &local_ray, const Geom &geom)
    {
        { to_intersections(geom, local_intersect(shape, local_ray)) } -> std::same_as<std::vector<Intersection> >;
    };

    // Converts a world-space ray into `geom`'s object space (by its cached inverse transform), dispatches to the
    // shape's own local_intersect, and pairs every t value with `geom`.
    std::vector<Intersection> intersect(const Geom &geom, const Ray &ray);

    std::optional<Intersection> hit(const std::vector<Intersection> &intersections);

    // Bounding boxes (BoundingBox itself is in Geom.hpp, because Group caches one). Every local_bounds returns the
    // shape's box in its own object space, or std::nullopt if the shape is unbounded (planes, and cylinders or
    // cones without both a minimum and a maximum).
    template<typename S>
    concept Boundable = requires(const S &shape)
    {
        { local_bounds(shape) } -> std::same_as<std::optional<BoundingBox> >;
    };

    std::optional<BoundingBox> local_bounds(const Sphere &);

    std::optional<BoundingBox> local_bounds(const Plane &);

    std::optional<BoundingBox> local_bounds(const Cube &);

    std::optional<BoundingBox> local_bounds(const Cylinder &cylinder);

    std::optional<BoundingBox> local_bounds(const Cone &cone);

    // The union of the children's boxes in the group's space; nullopt if any child is unbounded. Computed from the
    // children every time (it doesn't read the cache); refresh_bounds() stores it.
    std::optional<BoundingBox> local_bounds(const Group &group);

    std::optional<BoundingBox> local_bounds(const Triangle &triangle);

    std::optional<BoundingBox> local_bounds(const SmoothTriangle &smooth_triangle);

    std::optional<BoundingBox> local_bounds(const Csg &csg);

    std::optional<BoundingBox> local_bounds(const TestShape &);

    // The box around all 8 transformed corners. An empty box stays empty.
    BoundingBox transform(const BoundingBox &box, const Matrix<double> &m);

    // A geom's bounds in its parent's space: its local bounds through its own transform. nullopt if unbounded.
    std::optional<BoundingBox> parent_space_bounds(const Geom &geom);

    // Whether the ray passes through the box (slab method). Never true for an empty box.
    bool intersects(const BoundingBox &box, const Ray &ray);

    // Recomputes and caches the bounds of `geom` and every group below it. Call once the scene is built, and again
    // after changing any child's transform or adding children.
    void refresh_bounds(Geom &geom);

    bool intersection_allowed(Csg::Operation op, bool lhit, bool inl, bool inr);

    std::vector<Intersection> filter_intersections(const Csg &csg, const std::vector<Intersection> &intersections);


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
                                      const std::vector<Intersection> &intersections);

    Colour shade_hit(const World &world, const Computations &computations, uint8_t remaining);

    Matrix<double> view_transform(const Point &from, const Point &to, const Vector &up);
}

#endif //THE_RAYTRACER_CHALLENGE_INTERSECT_HPP
