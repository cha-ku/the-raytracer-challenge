#ifndef THE_RAYTRACER_CHALLENGE_GEOM_HPP
#define THE_RAYTRACER_CHALLENGE_GEOM_HPP

#include "Material.hpp"
#include "MatrixImpl.hpp"
#include "Ray.hpp"
#include "Vector.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace raytracer {
    // Pure geometry: each shape type only describes its own object-space surface. Where it sits in the scene
    // (transform, parent) and how it looks (material) belong to the Geom that holds it.
    struct Sphere {
    };

    struct Plane {
    };

    struct Cube {
    };

    struct Cylinder {
        // y-axis min and max values
        float minimum{-std::numeric_limits<float>::infinity()};
        float maximum{std::numeric_limits<float>::infinity()};
        bool closed{false};
    };

    struct Cone {
        // y-axis min and max values
        float minimum{-std::numeric_limits<float>::infinity()};
        float maximum{std::numeric_limits<float>::infinity()};
        bool closed{false};
    };

    // An axis-aligned box. Default-constructed it is empty (min = +inf, max = -inf), so the first add() replaces
    // both corners. It only ever holds finite or empty boxes: shapes that extend to infinity have no box at all
    // (local_bounds returns std::nullopt), so infinities never reach the arithmetic in transform().
    struct BoundingBox {
        Point min{
            .x = std::numeric_limits<float>::infinity(), .y = std::numeric_limits<float>::infinity(),
            .z = std::numeric_limits<float>::infinity()
        };
        Point max{
            .x = -std::numeric_limits<float>::infinity(), .y = -std::numeric_limits<float>::infinity(),
            .z = -std::numeric_limits<float>::infinity()
        };

        [[nodiscard]] bool empty() const { return min.x > max.x; }

        void add(const Point &p) {
            min = {.x = std::min(min.x, p.x), .y = std::min(min.y, p.y), .z = std::min(min.z, p.z)};
            max = {.x = std::max(max.x, p.x), .y = std::max(max.y, p.y), .z = std::max(max.z, p.z)};
        }

        void add(const BoundingBox &box) {
            if (!box.empty()) {
                add(box.min);
                add(box.max);
            }
        }

        [[nodiscard]] bool contains(const Point &point) const;
        [[nodiscard]] bool contains(const BoundingBox &box) const;
    };

    std::pair<BoundingBox, BoundingBox> split_bounds(const BoundingBox& box);

    struct Group {
        std::vector<Geom *> children;
        // cached bounds box
        std::optional<BoundingBox> bounds;
    };

    // Makes `child` a child of `group`. It doesn't remove `child` from a previous group: a child in two groups would
    // be intersected twice, so remove it from the old group first.
    void add_child(Geom &group, Geom &child);

    struct Triangle {
        Point v1;
        Point v2;
        Point v3;
        Vector e1;
        Vector e2;
        Vector normal;

        explicit Triangle(const Point &v1, const Point &v2, const Point &v3) : v1{v1}, v2{v2}, v3{v3}, e1{v2 - v1},
                                                                               e2{v3 - v1},
                                                                               normal{
                                                                                   Vector::normalize(
                                                                                       Vector::cross(e2, e1))
                                                                               } {
        }
    };

    struct SmoothTriangle {
        Point p1{};
        Point p2{};
        Point p3{};
        Vector n1{};
        Vector n2{};
        Vector n3{};

        // explicit SmoothTriangle(const Point &p1, const Point &p2, const Point &p3, const Vector &n1, const Vector &n2,
        //                         const Vector &n3) : p1{p1}, p2{p2}, p3{p3}, n1{n1}, n2{n2}, n3{n3} {}
    };

    struct Csg {
        enum class Operation {
            Union,
            Intersection,
            Difference,
        };
        Operation op;
        Geom *left;
        Geom *right;
        // cached bounds box
        std::optional<BoundingBox> bounds;

        explicit Csg(const Operation op, Geom &left, Geom &right) : op{op}, left{&left}, right{&right} {}
    };

    // A minimal shape used only to exercise Geom's own behaviour (transform/material) in tests. Like TestPattern
    // it lives in production code so it can be part of AnyShape; the name signals it is for tests only.
    struct TestShape {
        // Records the local-space ray passed to local_intersect, so tests can verify intersect() converted the
        // world-space ray into object space. mutable because recording it is an observation, not a change to the
        // shape, and local_intersect receives the shape by const reference.
        mutable Ray saved_ray{};
    };

    // The closed set of concrete shape types. Adding a new shape means adding it here, plus its own
    // local_intersect/local_normal_at overloads in Intersect.hpp.
    using AnyShape = std::variant<Sphere, Plane, Cube, Cylinder, Cone, Group, Triangle, SmoothTriangle, Csg, TestShape>;

    // A node in the scene: a shape plus where it is (transform, parent) and how it looks (material).
    //
    // Neither copyable nor movable. Other Geoms (children, via `parent`) and Intersections hold pointers to a
    // Geom, so its address must never change; deleting copy and move turns any accidental relocation into a
    // compile error. The World owns its Geoms through std::unique_ptr; tests can keep them as locals. Factory
    // functions like make_sphere() still work because returning a prvalue is guaranteed copy elision.
    // struct Group;
    struct Geom {
        AnyShape shape;
        Material material{};
        Geom *parent{nullptr};

        explicit Geom(AnyShape s) : shape{std::move(s)} {
            if (const auto *csg = std::get_if<Csg>(&shape)) {
                csg->left->parent = this;
                csg->right->parent = this;
            }
        }

        // For factories that need to return a Geom with a material already set: a named local Geom can't be
        // returned (that would need a move), but a prvalue built from this can
        Geom(AnyShape s, Material m) : Geom{std::move(s)} {
            material = std::move(m);
        }

        Geom(const Geom &) = delete;

        Geom &operator=(const Geom &) = delete;

        // Also caches the inverse, so intersect/normal_at don't invert a matrix for every ray.
        // Throws std::invalid_argument if the transform isn't invertible.
        void set_transform(const Matrix<double> &t);

        [[nodiscard]] const Matrix<double> &transform() const { return m_transform; }

        [[nodiscard]] const Matrix<double> &inverse_transform() const { return m_inverse; }

    private:
        Matrix<double> m_transform{Matrix<double>::identity(4)};
        Matrix<double> m_inverse{Matrix<double>::identity(4)};
    };

    inline Geom make_sphere() { return Geom{Sphere{}}; }

    inline Geom make_plane() { return Geom{Plane{}}; }

    inline Geom make_cube() { return Geom{Cube{}}; }

    inline Geom make_cylinder() { return Geom{Cylinder{}}; }

    inline Geom make_cone() { return Geom{Cone{}}; }

    inline Geom make_group() { return Geom{Group{}}; }

    inline Geom make_triangle(const Point &p1, const Point &p2, const Point &p3) { return Geom{Triangle{p1, p2, p3}}; }

    inline Geom make_csg(const Csg::Operation &op, Geom &left, Geom &right) {
        return Geom{Csg{op, left, right}};
    }

    // Converts a world-space point into the geom's object space, going through every parent's transform first
    Point world_to_object(const Geom &geom, const Point &world_point);

    // Converts an object-space normal into world space (by the inverse transpose of each transform), going out
    // through every parent. Returns a normalised vector.
    Vector normal_to_world(const Geom &geom, const Vector &object_normal);

    // The colour of `pattern` at a world-space point on `geom`: world -> object space (through any parents), then
    // object -> pattern space
    template<Patternable P>
    Colour pattern_at_shape(const P &pattern, const Geom &geom, const Point &world_point) {
        const auto pattern_transform_inverse{inverse(pattern.m_transform)};
        if (!pattern_transform_inverse.has_value()) {
            return {};
        }
        const Point object_point{world_to_object(geom, world_point)};
        const Point pattern_point{transform(object_point, pattern_transform_inverse.value())};
        return pattern_at(pattern, pattern_point);
    }
}

#endif //THE_RAYTRACER_CHALLENGE_GEOM_HPP
