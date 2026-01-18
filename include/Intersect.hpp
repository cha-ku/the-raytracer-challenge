//
// Created by chaku on 28/12/2025.
//

#ifndef THE_RAYTRACER_CHALLENGE_INTERSECT_HPP
#define THE_RAYTRACER_CHALLENGE_INTERSECT_HPP

#include "Point.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include <vector>
#include <optional>

namespace raytracer {
    static uint32_t sphere_id{0};

    struct Ray {
        Point origin{};
        Vector direction{};
    };

    Point position(const Ray &ray, float distance);

    Ray transform(const Ray &ray, const Container<double> &matrix);

    struct Sphere {
        uint32_t id;
        Container<double> transform{Container<double>::identity(4)};

        Sphere() = delete;

        explicit Sphere(const uint32_t id) : id(id) {};

        static Sphere make_sphere();

        bool operator==(const Sphere& other) const { return id == other.id; }
    };

    void set_transform(Sphere& s, const Container<double> &t);

    struct Intersection {
        Sphere object;
        float t{};
    };

    template<typename... Args>
    std::vector<Intersection> intersections(Args&&... args) {
        return std::vector<Intersection>{std::forward<Args>(args)...};
    }

    std::vector<Intersection> intersect(const Sphere &sphere, const Ray &ray);

    std::optional<Intersection> hit(const std::vector<Intersection> &intersections);

}

#endif //THE_RAYTRACER_CHALLENGE_INTERSECT_HPP