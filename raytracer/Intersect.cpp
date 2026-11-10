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

    Point to_object_space(const Point &point, const Matrix<double> &inverse_transform) {
        return transform(point, inverse_transform);
    }

    Vector to_world_normal(const Vector &object_normal, const Matrix<double> &inverse_transform) {
        const Matrix world_normal_matrix{
            multiply(transpose(inverse_transform), make_matrix(object_normal))
        };
        const Vector world_normal{
            .x = static_cast<float>(world_normal_matrix[0, 0]),
            .y = static_cast<float>(world_normal_matrix[1, 0]),
            .z = static_cast<float>(world_normal_matrix[2, 0])
        };
        return Vector::normalize(world_normal);
    }

    Vector local_normal_at(const Sphere &sphere, const Point &local_point) {
        return local_point - Point{.x = 0, .y = 0, .z = 0};
    }

    Vector local_normal_at(const Plane &plane, const Point &local_point) {
        return Vector{.x = 0, .y = 1, .z = 0};
    }

    Vector local_normal_at(const Cube &cube, const Point &local_point) {
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


    std::vector<Intersection> local_intersect(const Sphere &sphere, const Ray &local_ray) {
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
        return {{.object = sphere, .t = t1}, {.object = sphere, .t = t2}};
    }

    std::vector<Intersection> local_intersect(const Plane &plane, const Ray &local_ray) {
        if (std::abs(local_ray.direction.y) < utils::EPSILON) {
            return {};
        }
        const float t{-local_ray.origin.y / local_ray.direction.y};
        return {{.object = plane, .t = t}};
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

    std::vector<Intersection> local_intersect(const Cube &cube, const Ray &local_ray) {
        // cube is made up of 6 planes of size 2 that intersect each other
        // xy planes at z = 1 and z = -1
        // xz planes at y = 1 and y = -1
        // yz planes at x = 1 and x = -1
        const auto& [xtmin, xtmax] = check_axis(local_ray.direction.x, local_ray.origin.x);
        const auto& [ytmin, ytmax] = check_axis(local_ray.direction.y, local_ray.origin.y);
        auto tmin = std::max(xtmin, ytmin);
        auto tmax = std::min(xtmax, ytmax);
        // check if ray passes outside the xy boundaries, if it does, then return empty because no z value can fix it
        if (tmin > tmax) return std::vector<Intersection>{};
        const auto& [ztmin, ztmax] = check_axis(local_ray.direction.z, local_ray.origin.z);
        tmin = std::max({tmin, ztmin});
        tmax = std::min({tmax, ztmax});
        return tmin > tmax
                   ? std::vector<Intersection>{}
                   : std::vector{Intersection{.object = cube, .t = tmin}, Intersection{.object = cube, .t = tmax}};
    }

    // helper function to reduce duplication. Checks to see if the intersection at `t` is within radius of 1 (the radius of cylinders) from the y axis
    static bool check_cap(const Ray &ray, const float &t) {
        const auto x = ray.origin.x + ray.direction.x * t;
        const auto z = ray.origin.z + ray.direction.z * t;
        return utils::square(x) + utils::square(z) <= 1 + utils::EPSILON;
    }

    static void intersect_caps(const Cylinder &cylinder, const Ray &ray, std::vector<Intersection> &intersections) {
        // doesn't matter if cylinder is closed or ray is parallel to y
        if (!cylinder.closed or std::abs(ray.direction.y) < utils::EPSILON) {
            return;
        }
        // check for an intersection with the lower end cap by intersecting the ray with the plane at y=cyl.minimum
        float t = (cylinder.minimum - ray.origin.y) / ray.direction.y;
        if (check_cap(ray, t)) {
            intersections.emplace_back(cylinder, t);
        }
        // check for an intersection with the upper end cap by intersecting the ray with the plane at y=cyl.maximum
        t = (cylinder.maximum - ray.origin.y) / ray.direction.y;
        if (check_cap(ray, t)) {
            intersections.emplace_back(cylinder, t);
        }
    }

    std::vector<Intersection> local_intersect(const Cylinder &cylinder, const Ray &local_ray) {
        const auto a = utils::square(local_ray.direction.x) + utils::square(local_ray.direction.z);
        std::vector<Intersection> intersections;
        // ray is parallel to y axis, only check caps
        if (a > utils::EPSILON) {
            const auto b = 2 * local_ray.origin.x * local_ray.direction.x + 2 * local_ray.origin.z * local_ray.direction.z;
            const auto c = utils::square(local_ray.origin.x) + utils::square(local_ray.origin.z) - 1;
            // same as sphere
            const auto discriminant{utils::square(b) - (4 * a * c)};
            if (discriminant < 0) {
                return {};
            }
            float t0{(-b - std::sqrt(discriminant)) / (2 * a)};
            float t1{(-b + std::sqrt(discriminant)) / (2 * a)};
            if (t0 > t1) {
                std::swap(t0, t1);
            }
            const float y0 = local_ray.origin.y + local_ray.direction.y * t0;
            if (cylinder.minimum < y0 && y0 < cylinder.maximum) {
                intersections.emplace_back(cylinder, t0);
            }
            const float y1 = local_ray.origin.y + local_ray.direction.y * t1;
            if (cylinder.maximum > y1 && y1 > cylinder.minimum) {
                intersections.emplace_back(cylinder, t1);
            }
        }
        intersect_caps(cylinder, local_ray, intersections);
        return intersections;
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
        normal_vector{std::visit([this](const auto &shape) { return normal_at(shape, point); }, object)},
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
        // Objects the ray is currently inside, innermost last. Pointers into `intersections` (not copies), so
        // there's no per-intersection allocation; they stay valid for the whole loop since the vector isn't modified
        std::vector<const AnyShape *> shape_container;
        for (const auto &ixn: intersections) {
            // check if intersection is hit
            if (ixn == intersection) {
                comp.n1 = shape_container.empty() ? 1.0f : material_of(*shape_container.back()).refractive_index;
            }
            const auto it = std::ranges::find_if(shape_container, [&ixn](const AnyShape *shape) {
                return *shape == ixn.object;
            });
            // intersection at the exit of the material - remove it
            if (it != shape_container.end()) {
                shape_container.erase(it);
            } else {
                // intersection when entering the material - add it
                shape_container.push_back(&ixn.object);
            }
            if (ixn == intersection) {
                comp.n2 = shape_container.empty() ? 1.0f : material_of(*shape_container.back()).refractive_index;
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
            lighting(as_shape(computations.object), material_of(computations.object), world.light.value(),
                     computations.over_point,
                     computations.eye_vector, computations.normal_vector, world.is_shadowed(computations.over_point))
        };
        --remaining;
        const Colour reflected{World::reflected_colour(world, computations, remaining)};
        const Colour refracted{World::refracted_colour(world, computations, remaining)};

        const auto& mat{material_of(computations.object)};
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
