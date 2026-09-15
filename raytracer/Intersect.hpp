//
// Created by chaku on 28/12/2025.
//

#ifndef THE_RAYTRACER_CHALLENGE_INTERSECT_HPP
#define THE_RAYTRACER_CHALLENGE_INTERSECT_HPP

#include "Point.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Material.hpp"

#include <vector>
#include <optional>

namespace raytracer {
    struct World;
    static uint32_t sphere_id{0};

    struct Ray {
        Point origin{};
        Vector direction{};
    };

    Point position(const Ray &ray, float distance);

    Ray transform(const Ray &ray, const Matrix<double> &matrix);

    struct Sphere {
        uint32_t id;
        Matrix<double> transform{Matrix<double>::identity(4)};
        Material material;

        Sphere() = delete;

        explicit Sphere(const uint32_t id) : id(id) {};

        static Sphere make_sphere();

        void set_transform(const Matrix<double> &t);

        bool operator==(const Sphere& other) const { return id == other.id; }

        static Vector normal_at(const Point& point) ;
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