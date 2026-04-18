//
// Created by chaku on 17/03/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_WORLD_HPP
#define THE_RAYTRACER_CHALLENGE_WORLD_HPP

#include <optional>
#include "Intersect.hpp"
#include "Light.hpp"
#include "Camera.hpp"

namespace raytracer {
    struct World {
        std::vector<Sphere> objects;
        std::optional<PointLight> light;

        static World create_default_world();

        static std::vector<Intersection> intersect_world(const World& world, const Ray& ray);

        static Colour colour_at(const World& world, const Ray& ray);

        static Ray ray_for_pixel(const Camera& camera, double px, double py);
    };
}

#endif //THE_RAYTRACER_CHALLENGE_WORLD_HPP