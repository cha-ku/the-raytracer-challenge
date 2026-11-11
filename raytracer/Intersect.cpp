//
// Created by chaku on 28/12/2025.
//

#include "Intersect.hpp"

#include <algorithm>

#include "MatrixImpl.hpp"
#include "World.hpp"
#include "Utils.hpp"

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
        const auto& maxc = std::max({std::abs(local_point.x), std::abs(local_point.y), std::abs(local_point.z)});
        if (maxc == std::abs(local_point.x)) {
            return {.x=local_point.x, .y=0, .z=0};
        }
        if (maxc == std::abs(local_point.y)) {
            return {.x = 0, .y=local_point.y, .z=0};
        }
        return {.x = 0, .y=0, .z=local_point.z};
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

    // helper for local_intersect - Cube
    static std::pair<float, float> check_axis(const float& direction_component, const float& origin_component) {
        // generalized formula for ray-plane intersection, the plane
        const auto tmin_numerator = -1.f - origin_component;
        const auto tmax_numerator = 1.f - origin_component;
        float tmin, tmax;
        if (std::abs(direction_component) < utils::EPSILON) {
            tmin = tmin_numerator * std::numeric_limits<float>::infinity();
            tmax = tmax_numerator * std::numeric_limits<float>::infinity();
        }
        else {
            tmin = tmin_numerator/direction_component;
            tmax = tmax_numerator/direction_component;
        }
        return tmin > tmax ? std::make_pair(tmax, tmin) : std::make_pair(tmin, tmax);
    }

    std::vector<float> local_intersect(const Cube &, const Ray &local_ray) {
        // cube is made up of 6 planes of size 2 that intersect each other
        // xy planes at z = 1 and z = -1
        // xz planes at y = 1 and y = -1
        // yz planes at x = 1 and x = -1
        const auto& [xtmin, xtmax] = check_axis(local_ray.direction.x, local_ray.origin.x);
        const auto& [ytmin, ytmax] = check_axis(local_ray.direction.y, local_ray.origin.y);
        auto tmin = std::max(xtmin, ytmin);
        auto tmax = std::min(xtmax, ytmax);
        // check if ray passes outside the xy boundaries, if it does, then return empty because no z value can fix it
        if (tmin > tmax) return {};
        const auto& [ztmin, ztmax] = check_axis(local_ray.direction.z, local_ray.origin.z);
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
    template <typename S>
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

    std::vector<float> local_intersect(const Cylinder &cylinder, const Ray &local_ray) {
        const auto a = utils::square(local_ray.direction.x) + utils::square(local_ray.direction.z);
        std::vector<float> intersections;
        // ray is parallel to y axis, only check caps
        if (a > utils::EPSILON) {
            const auto b = 2 * local_ray.origin.x * local_ray.direction.x + 2 * local_ray.origin.z * local_ray.direction.z;
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
            const float t = -c/(2 * b);
            intersections.push_back(t);
        }
        else {
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


    std::vector<float> local_intersect(const TestShape &shape, const Ray &local_ray) {
        shape.saved_ray = local_ray;
        return {};
    }

    Vector local_normal_at(const TestShape &, const Point &local_point) {
        return Vector{.x = local_point.x, .y = local_point.y, .z = local_point.z};
    }

    std::vector<Intersection> intersect(const Geom &geom, const Ray &ray) {
        const Ray local_ray{transform(ray, geom.inverse_transform())};
        const std::vector<float> ts{
            std::visit([&local_ray]<Intersectable S>(const S &shape) { return local_intersect(shape, local_ray); },
                       geom.shape)
        };
        std::vector<Intersection> result;
        result.reserve(ts.size());
        for (const float t: ts) {
            result.push_back({.object = &geom, .t = t});
        }
        return result;
    }

    Vector normal_at(const Geom &geom, const Point &world_point) {
        const Point local_point{world_to_object(geom, world_point)};
        const Vector local_normal{
            std::visit([&local_point]<Normalable S>(const S &shape) { return local_normal_at(shape, local_point); },
                       geom.shape)
        };
        return normal_to_world(geom, local_normal);
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
        normal_vector{normal_at(*object, point)},
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
                                      const std::vector<Intersection>& intersections) {
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

        const auto& mat{computations.object->material};
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
