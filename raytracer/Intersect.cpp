//
// Created by chaku on 28/12/2025.
//

#include "Intersect.hpp"

#include <algorithm>

#include "MatrixImpl.hpp"
#include "World.hpp"
#include "Utils.hpp"

#include <cmath>

namespace raytracer {
    Point position(const Ray &ray, const float distance) {
        return ray.origin + ray.direction * distance;
    }

    Ray transform(const Ray &ray, const Matrix<double> &matrix) {
        const Matrix<double> origin_matrix{make_matrix(Point{.x = ray.origin.x, .y = ray.origin.y, .z = ray.origin.z})};
        const Matrix<double> direction_matrix{
            4, 1, std::vector{
                static_cast<double>(ray.direction.x),
                static_cast<double>(ray.direction.y),
                static_cast<double>(ray.direction.z),
                0.0
            }
        };

        const Matrix<double> new_origin{multiply(matrix, origin_matrix)};
        const Matrix<double> new_direction{multiply(matrix, direction_matrix)};

        return Ray{
            .origin = Point{
                .x = static_cast<float>(new_origin[0, 0]),
                .y = static_cast<float>(new_origin[1, 0]),
                .z = static_cast<float>(new_origin[2, 0])
            },
            .direction = Vector{
                .x = static_cast<float>(new_direction[0, 0]),
                .y = static_cast<float>(new_direction[1, 0]),
                .z = static_cast<float>(new_direction[2, 0])
            }
        };
    }

    Vector local_normal_at(const Sphere &, const Point &local_point) {
        return local_point - Point{.x = 0, .y = 0, .z = 0};
    }

    Vector local_normal_at(const Plane &, const Point &) {
        return Vector{.x = 0, .y = 1, .z = 0};
    }

    Vector local_normal_at(const Cube &, const Point &local_point) {
        // The face is always the one matching the component whose absolute value is a 1!
        // could also use utils::equal to see which component is closest to 1.f, but wanna avoid floating point comparisons
        const auto &maxc = std::max({std::abs(local_point.x), std::abs(local_point.y), std::abs(local_point.z)});
        if (maxc == std::abs(local_point.x)) {
            return {.x = local_point.x, .y = 0, .z = 0};
        }
        if (maxc == std::abs(local_point.y)) {
            return {.x = 0, .y = local_point.y, .z = 0};
        }
        return {.x = 0, .y = 0, .z = local_point.z};
    }

    Vector local_normal_at(const Cylinder &cylinder, const Point &local_point) {
        const auto distance = utils::square(local_point.x) + utils::square(local_point.z);
        // if point is on the end planes, return normal to the planes
        if (distance < 1.f && local_point.y >= cylinder.maximum - utils::EPSILON) {
            return {.x = 0, .y = 1, .z = 0};
        }
        if (distance < 1.f && local_point.y <= cylinder.minimum + utils::EPSILON) {
            return {.x = 0, .y = -1, .z = 0};
        }
        return {.x = local_point.x, .y = 0, .z = local_point.z};
    }

    Vector local_normal_at(const Cone &, const Point &local_point) {
        const auto y = std::sqrt(utils::square(local_point.x) + utils::square(local_point.z));
        return {.x = local_point.x, .y = local_point.y > 0.f ? -y : y, .z = local_point.z};
    }

    Vector local_normal_at(const Group &, const Point &) {
        throw std::logic_error("local_normal_at called on a Group: a group has no surface, ask its children");
    }

    Vector local_normal_at(const Triangle &triangle, const Point &) {
        return triangle.normal;
    }

    Vector local_normal_at(const SmoothTriangle &smooth_triangle, const Point &, const Intersection &hit) {
        const float u{*hit.u};
        const float v{*hit.v};
        return smooth_triangle.n2 * u + smooth_triangle.n3 * v + smooth_triangle.n1 * (1 - u - v);
    }

    Vector local_normal_at(const Csg &, const Point &) {
        throw std::logic_error("local_normal_at called on a Csg: ask its children");
    }

    std::vector<float> local_intersect(const Sphere &, const Ray &local_ray) {
        const auto &[origin, direction] = local_ray;

        const Vector sphere_to_ray{origin - Point{.x = 0, .y = 0, .z = 0}};
        const float a{Vector::dot(direction, direction)};
        const float b{2 * Vector::dot(direction, sphere_to_ray)};
        const float c{Vector::dot(sphere_to_ray, sphere_to_ray) - 1};
        const float discriminant{utils::square(b) - (4 * a * c)};
        if (discriminant < 0) {
            return {};
        }
        const float t1{(-b - std::sqrt(discriminant)) / (2 * a)};
        const float t2{(-b + std::sqrt(discriminant)) / (2 * a)};
        return {t1, t2};
    }

    std::vector<float> local_intersect(const Plane &, const Ray &local_ray) {
        if (std::abs(local_ray.direction.y) < utils::EPSILON) {
            return {};
        }
        const float t{-local_ray.origin.y / local_ray.direction.y};
        return {t};
    }

    // ray-plane intersection
    static std::pair<float, float> check_axis(const float direction_component, const float origin_component,
                                              const float min = -1.f, const float max = 1.f) {
        const auto tmin_numerator = min - origin_component;
        const auto tmax_numerator = max - origin_component;
        float tmin, tmax;
        if (std::abs(direction_component) < utils::EPSILON) {
            tmin = tmin_numerator * std::numeric_limits<float>::infinity();
            tmax = tmax_numerator * std::numeric_limits<float>::infinity();
        } else {
            tmin = tmin_numerator / direction_component;
            tmax = tmax_numerator / direction_component;
        }
        return tmin > tmax ? std::make_pair(tmax, tmin) : std::make_pair(tmin, tmax);
    }

    std::vector<float> local_intersect(const Cube &, const Ray &local_ray) {
        // cube is made up of 6 planes of size 2 that intersect each other
        // xy planes at z = 1 and z = -1
        // xz planes at y = 1 and y = -1
        // yz planes at x = 1 and x = -1
        const auto &[xtmin, xtmax] = check_axis(local_ray.direction.x, local_ray.origin.x);
        const auto &[ytmin, ytmax] = check_axis(local_ray.direction.y, local_ray.origin.y);
        auto tmin = std::max(xtmin, ytmin);
        auto tmax = std::min(xtmax, ytmax);
        // check if ray passes outside the xy boundaries, if it does, then return empty because no z value can fix it
        if (tmin > tmax) return {};
        const auto &[ztmin, ztmax] = check_axis(local_ray.direction.z, local_ray.origin.z);
        tmin = std::max({tmin, ztmin});
        tmax = std::min({tmax, ztmax});
        if (tmin > tmax) return {};
        return {tmin, tmax};
    }

    // helper for intersect_caps. Checks whether the ray at `t` is within `radius` of the y axis, i.e. inside the cap
    static bool check_cap(const Ray &ray, const float t, const float radius) {
        const auto x = ray.origin.x + ray.direction.x * t;
        const auto z = ray.origin.z + ray.direction.z * t;
        return utils::square(x) + utils::square(z) <= utils::square(radius) + utils::EPSILON;
    }

    // Adds intersections with the end caps of a closed cylinder or cone. A cylinder's caps always have radius 1;
    // a cone's radius at height y is |y|, so each cap's radius comes from the height it sits at
    template<typename S>
    static void intersect_caps(const S &shape, const Ray &ray, std::vector<float> &intersections) {
        // nothing to do if the shape is open, or if the ray is parallel to the caps and can never cross them
        if (!shape.closed or std::abs(ray.direction.y) < utils::EPSILON) {
            return;
        }
        // check for an intersection with the lower end cap by intersecting the ray with the plane at y = minimum
        float t = (shape.minimum - ray.origin.y) / ray.direction.y;
        if (check_cap(ray, t, std::is_same_v<S, Cylinder> ? 1.f : std::abs(shape.minimum))) {
            intersections.push_back(t);
        }
        // check for an intersection with the upper end cap by intersecting the ray with the plane at y = maximum
        t = (shape.maximum - ray.origin.y) / ray.direction.y;
        if (check_cap(ray, t, std::is_same_v<S, Cylinder> ? 1.f : std::abs(shape.maximum))) {
            intersections.push_back(t);
        }
    }

    Vector local_normal_at(const TestShape &, const Point &local_point) {
        return Vector{.x = local_point.x, .y = local_point.y, .z = local_point.z};
    }

    Vector normal_at(const Geom &geom, const Point &world_point, const Intersection &hit) {
        const Point local_point{world_to_object(geom, world_point)};
        const Vector local_normal{
            std::visit([&local_point, &hit]<Normalable S>(const S &shape) {
                           return local_normal_at(shape, local_point, hit);
                       },
                       geom.shape)
        };
        return normal_to_world(geom, local_normal);
    }

    std::vector<float> local_intersect(const Cylinder &cylinder, const Ray &local_ray) {
        const auto a = utils::square(local_ray.direction.x) + utils::square(local_ray.direction.z);
        std::vector<float> intersections;
        // ray is parallel to y axis, only check caps
        if (a > utils::EPSILON) {
            const auto b = 2 * local_ray.origin.x * local_ray.direction.x + 2 * local_ray.origin.z * local_ray.direction
                           .z;
            const auto c = utils::square(local_ray.origin.x) + utils::square(local_ray.origin.z) - 1;
            // same as sphere
            const auto discriminant{utils::square(b) - (4 * a * c)};
            // infinitesimally small values are OK
            if (discriminant < -utils::EPSILON) {
                return intersections;
            }
            const float root{std::sqrt(std::max(discriminant, 0.f))};
            float t0{(-b - root) / (2 * a)};
            float t1{(-b + root) / (2 * a)};
            if (t0 > t1) {
                std::swap(t0, t1);
            }
            const float y0 = local_ray.origin.y + local_ray.direction.y * t0;
            if (cylinder.minimum < y0 && y0 < cylinder.maximum) {
                intersections.push_back(t0);
            }
            const float y1 = local_ray.origin.y + local_ray.direction.y * t1;
            if (cylinder.maximum > y1 && y1 > cylinder.minimum) {
                intersections.push_back(t1);
            }
        }
        intersect_caps(cylinder, local_ray, intersections);
        return intersections;
    }

    std::vector<float> local_intersect(const Cone &cone, const Ray &local_ray) {
        std::vector<float> intersections;
        const auto [dx, dy, dz] = local_ray.direction;
        const auto origin = local_ray.origin;
        const auto a = utils::square(dx) - utils::square(dy) + utils::square(dz);
        const auto b = 2 * origin.x * dx - 2 * origin.y * dy + 2 * origin.z * dz;
        const auto c = utils::square(origin.x) - utils::square(origin.y) + utils::square(origin.z);
        if (std::abs(a) < utils::EPSILON) {
            if (std::abs(b) < utils::EPSILON) {
                return intersections;
            }
            const float t = -c / (2 * b);
            intersections.push_back(t);
        } else {
            const auto discriminant{utils::square(b) - (4 * a * c)};
            // infinitesimally small values are OK
            if (discriminant < -utils::EPSILON) {
                return intersections;
            }
            const float root{std::sqrt(std::max(discriminant, 0.f))};
            float t0{(-b - root) / (2 * a)};
            float t1{(-b + root) / (2 * a)};
            if (t0 > t1) {
                std::swap(t0, t1);
            }
            const float y0 = local_ray.origin.y + local_ray.direction.y * t0;
            if (cone.minimum < y0 && y0 < cone.maximum) {
                intersections.push_back(t0);
            }
            const float y1 = local_ray.origin.y + local_ray.direction.y * t1;
            if (cone.maximum > y1 && y1 > cone.minimum) {
                intersections.push_back(t1);
            }
        }
        intersect_caps(cone, local_ray, intersections);
        return intersections;
    }


    std::vector<Intersection> local_intersect(const Group &group, const Ray &local_ray) {
        // ray does not intersect group bounds
        if (group.bounds && !intersects(*group.bounds, local_ray)) {
            return {};
        }
        std::vector<Intersection> intersections;
        for (const Geom *child: group.children) {
            intersections.append_range(intersect(*child, local_ray));
        }
        std::ranges::sort(intersections, {}, &Intersection::t);
        return intersections;
    }

    namespace {
        // Möller-Trumbore algorithm
        std::optional<TriangleHit> moller_trumbore(const Point &p1, const Vector &e1, const Vector &e2,
                                                   const Ray &local_ray) {
            const Vector dir_cross_e2{Vector::cross(local_ray.direction, e2)};
            const float det{Vector::dot(e1, dir_cross_e2)};
            // ray parallel to the triangle. det grows with the triangle's size and the ray direction's length, so
            // compare it against those lengths: an absolute EPSILON rejected most of a fine mesh like the bunny
            const float scale{
                std::sqrt(Vector::dot(e1, e1) * Vector::dot(e2, e2) *
                          Vector::dot(local_ray.direction, local_ray.direction))
            };
            if (std::fabs(det) < utils::EPSILON * scale) return std::nullopt;
            const float f{1.0f / det};
            const Vector p1_to_origin{local_ray.origin - p1};
            const float u{f * Vector::dot(p1_to_origin, dir_cross_e2)};
            // ray misses the p1-p3 edge
            if (u < 0 || u > 1) return std::nullopt;
            const Vector origin_cross_e1{Vector::cross(p1_to_origin, e1)};
            const float v{f * Vector::dot(local_ray.direction, origin_cross_e1)};
            // ray misses the p1-p2 or p2-p3 edge
            if (v < 0 || u + v > 1) return std::nullopt;
            return TriangleHit{.t = f * Vector::dot(e2, origin_cross_e1), .u = u, .v = v};
        }
    }

    std::vector<float> local_intersect(const Triangle &triangle, const Ray &local_ray) {
        if (const auto hit{moller_trumbore(triangle.v1, triangle.e1, triangle.e2, local_ray)}) {
            return {hit->t};
        }
        return {};
    }

    std::optional<TriangleHit> local_intersect(const SmoothTriangle &smooth_triangle, const Ray &local_ray) {
        const Vector e1{smooth_triangle.p2 - smooth_triangle.p1};
        const Vector e2{smooth_triangle.p3 - smooth_triangle.p1};
        return moller_trumbore(smooth_triangle.p1, e1, e2, local_ray);
    }

    std::vector<Intersection> local_intersect(const Csg &csg, const Ray &local_ray) {
        std::vector<Intersection> intersections{intersect(*csg.left, local_ray)};
        intersections.append_range(intersect(*csg.right, local_ray));
        std::ranges::sort(intersections, {}, &Intersection::t);
        return filter_intersections(csg, intersections);
    }

    std::vector<float> local_intersect(const TestShape &shape, const Ray &local_ray) {
        shape.saved_ray = local_ray;
        return {};
    }

    std::vector<Intersection> intersect(const Geom &geom, const Ray &ray) {
        const Ray local_ray{transform(ray, geom.inverse_transform())};
        return std::visit([&local_ray, &geom]<Intersectable S>(const S &shape) {
            return to_intersections(geom, local_intersect(shape, local_ray));
        }, geom.shape);
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

    BoundingBox transform(const BoundingBox &box, const Matrix<double> &m) {
        // An empty box's +inf/-inf corners would turn into NaN; transforming nothing gives nothing
        if (box.empty()) {
            return box;
        }
        BoundingBox result;
        // transform 8 vertices of the bounding box
        for (const float x: {box.min.x, box.max.x})
            for (const float y: {box.min.y, box.max.y})
                for (const float z: {box.min.z, box.max.z})
                    result.add(transform(Point{.x = x, .y = y, .z = z}, m));
        return result;
    }

    std::optional<BoundingBox> parent_space_bounds(const Geom &geom) {
        const std::optional local{
            std::visit([]<Boundable S>(const S &shape) { return local_bounds(shape); }, geom.shape)
        };
        if (!local) {
            return std::nullopt;
        }
        return transform(*local, geom.transform());
    }

    bool intersects(const BoundingBox &box, const Ray &ray) {
        if (box.empty()) {
            return false;
        }
        const auto [xtmin, xtmax] = check_axis(ray.direction.x, ray.origin.x, box.min.x, box.max.x);
        const auto [ytmin, ytmax] = check_axis(ray.direction.y, ray.origin.y, box.min.y, box.max.y);
        const auto [ztmin, ztmax] = check_axis(ray.direction.z, ray.origin.z, box.min.z, box.max.z);
        return std::max({xtmin, ytmin, ztmin}) <= std::min({xtmax, ytmax, ztmax});
    }

    void refresh_bounds(Geom &geom) {
        auto *group = std::get_if<Group>(&geom.shape);
        if (group == nullptr) {
            return;
        }
        // Nested groups first, so each keeps its own cache for its own culling
        for (Geom *child: group->children) {
            refresh_bounds(*child);
        }
        group->bounds = local_bounds(*group);
    }

    bool intersection_allowed(const Csg::Operation op, const bool lhit, const bool inl, const bool inr) {
        switch (op) {
            case Csg::Operation::Union: return (lhit && !inr) || (!lhit && !inl);
            case Csg::Operation::Intersection: return (lhit && inr) || (!lhit && inl);
            case Csg::Operation::Difference: return (lhit && !inr) || (!lhit && inl);
        }
        return false;
    }

    namespace {
        bool includes(const Geom *geom, const Geom *other) {
            while (other) {
                if (geom == other) return true;
                other = other->parent;
            }
            return false;
        }
    }

    std::vector<Intersection> filter_intersections(const Csg &csg, const std::vector<Intersection> &intersections) {
        bool inl{false};
        bool inr{false};

        std::vector<Intersection> result;
        for (const auto &intersection: intersections) {
            const bool lhit{includes(csg.left, intersection.object)};
            if (intersection_allowed(csg.op, lhit, inl, inr)) {
                result.emplace_back(intersection);
            }
            if (lhit) {
                inl = !inl;
            }
            else {
                inr = !inr;
            }
        }
        return result;
    }

    std::optional<BoundingBox> local_bounds(const Group &group) {
        BoundingBox result;
        for (const Geom *child: group.children) {
            const std::optional child_box{parent_space_bounds(*child)};
            // One unbounded child makes the whole group unbounded
            if (!child_box) {
                return std::nullopt;
            }
            result.add(*child_box);
        }
        return result;
    }

    std::optional<BoundingBox> local_bounds(const Triangle &triangle) {
        BoundingBox result;
        result.add(triangle.v1);
        result.add(triangle.v2);
        result.add(triangle.v3);
        return result;
    }

    std::optional<BoundingBox> local_bounds(const SmoothTriangle &smooth_triangle) {
        BoundingBox result;
        result.add(smooth_triangle.p1);
        result.add(smooth_triangle.p2);
        result.add(smooth_triangle.p3);
        return result;
    }

    namespace {
        BoundingBox overlap(const BoundingBox &a, const BoundingBox &b) {
            const BoundingBox result{
                .min = {
                    .x = std::max(a.min.x, b.min.x), .y = std::max(a.min.y, b.min.y), .z = std::max(a.min.z, b.min.z)
                },
                .max = {
                    .x = std::min(a.max.x, b.max.x), .y = std::min(a.max.y, b.max.y), .z = std::min(a.max.z, b.max.z)
                }
            };
            // empty() only looks at x, so return the default empty box when any axis doesn't overlap
            if (result.min.x > result.max.x || result.min.y > result.max.y || result.min.z > result.max.z) {
                return {};
            }
            return result;
        }
    }

    std::optional<BoundingBox> local_bounds(const Csg &csg) {
        const std::optional left{parent_space_bounds(*csg.left)};
        // ignore right
        if (csg.op == Csg::Operation::Difference) {
            return left;
        }
        const std::optional right{parent_space_bounds(*csg.right)};
        if (csg.op == Csg::Operation::Intersection) {
            // Inside both shapes, so one bounded shape is enough to bound the result
            if (left && right) {
                return overlap(*left, *right);
            }
            return left ? left : right;
        }
        // Union: one unbounded shape makes the whole CSG unbounded
        if (!left || !right) {
            return std::nullopt;
        }
        BoundingBox result{*left};
        result.add(*right);
        return result;
    }

    std::optional<BoundingBox> local_bounds(const Sphere &) {
        return BoundingBox{.min = Point(-1, -1, -1), .max = Point(1, 1, 1)};
    }

    std::optional<BoundingBox> local_bounds(const Plane &) {
        // Infinite in x and z
        return std::nullopt;
    }

    std::optional<BoundingBox> local_bounds(const Cube &) {
        return BoundingBox{.min = Point(-1, -1, -1), .max = Point(1, 1, 1)};
    }

    std::optional<BoundingBox> local_bounds(const Cylinder &cylinder) {
        if (std::isinf(cylinder.minimum) || std::isinf(cylinder.maximum)) {
            return std::nullopt;
        }
        return BoundingBox{.min = Point(-1, cylinder.minimum, -1), .max = Point(1, cylinder.maximum, 1)};
    }

    std::optional<BoundingBox> local_bounds(const Cone &cone) {
        if (std::isinf(cone.minimum) || std::isinf(cone.maximum)) {
            return std::nullopt;
        }
        // The radius at height y is |y|, so whichever end is wider sets the x/z extent
        const auto r{std::max(std::abs(cone.maximum), std::abs(cone.minimum))};
        return BoundingBox{.min = Point(-r, cone.minimum, -r), .max = Point(r, cone.maximum, r)};
    }

    std::optional<BoundingBox> local_bounds(const TestShape &) {
        return BoundingBox{.min = Point(-1, -1, -1), .max = Point(1, 1, 1)};
    }

    Computations::Computations(const Intersection &intersection, const Ray &ray) : t{intersection.t},
        object{intersection.object}, point{position(ray, intersection.t)}, eye_vector{-ray.direction},
        normal_vector{normal_at(*object, point, intersection)},
        reflect_vector{Vector::reflect(ray.direction, normal_vector)} {
        if (Vector::dot(normal_vector, eye_vector) < 0) {
            inside = true;
            normal_vector = -normal_vector;
        } else {
            inside = false;
        }
        over_point = point + normal_vector * utils::EPSILON;
        under_point = point - normal_vector * utils::EPSILON;
    }

    Computations prepare_computations(const Intersection &intersection, const Ray &ray,
                                      const std::vector<Intersection> &intersections) {
        Computations comp{intersection, ray};
        // Objects the ray is currently inside, innermost last
        std::vector<const Geom *> shape_container;
        for (const auto &ixn: intersections) {
            // check if intersection is hit
            if (ixn == intersection) {
                comp.n1 = shape_container.empty() ? 1.0f : shape_container.back()->material.refractive_index;
            }
            const auto it = std::ranges::find(shape_container, ixn.object);
            // intersection at the exit of the material - remove it
            if (it != shape_container.end()) {
                shape_container.erase(it);
            } else {
                // intersection when entering the material - add it
                shape_container.push_back(ixn.object);
            }
            if (ixn == intersection) {
                comp.n2 = shape_container.empty() ? 1.0f : shape_container.back()->material.refractive_index;
                break;
            }
        }
        return comp;
    }

    Colour shade_hit(const World &world, const Computations &computations, uint8_t remaining) {
        if (remaining == 0) {
            return colours::black;
        }
        const Colour surface{
            lighting(*computations.object, computations.object->material, world.light.value(),
                     computations.over_point,
                     computations.eye_vector, computations.normal_vector, world.is_shadowed(computations.over_point))
        };
        --remaining;
        const Colour reflected{World::reflected_colour(world, computations, remaining)};
        const Colour refracted{World::refracted_colour(world, computations, remaining)};

        const auto &mat{computations.object->material};
        if (mat.reflective > 0.f && mat.transparency > 0.f) {
            const auto reflectance = World::schlick(computations);
            return surface + reflected * reflectance + refracted * (1.0f - reflectance);
        }
        return surface + reflected + refracted;
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
