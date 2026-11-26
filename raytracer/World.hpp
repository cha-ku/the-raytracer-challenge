//
// Created by chaku on 17/03/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_WORLD_HPP
#define THE_RAYTRACER_CHALLENGE_WORLD_HPP

#include <cstddef>
#include <memory>
#include <vector>
#include <optional>
#include "Intersect.hpp"
#include "Light.hpp"
#include "Camera.hpp"

namespace raytracer {
    struct World {
        // Owns every Geom in the scene. unique_ptr keeps each Geom at a fixed address, because Intersections
        // (and children's parent pointers) point at them. This makes World move-only.
        std::vector<std::unique_ptr<Geom> > objects;
        std::optional<PointLight> light;

        // Adds a new Geom holding `shape` and returns it, so the caller can set its transform and material
        Geom &add(AnyShape shape);

        // Divides every root object (see divide) into a bounding volume hierarchy and caches its bounds. Groups with at
        // least `threshold` children are split. Call once the scene is built.
        void build_bvh(std::size_t threshold);

        static World create_default_world();

        static std::vector<Intersection> intersect_world(const World &world, const Ray &ray);

        static Colour colour_at(const World &world, const Ray &ray, uint8_t remaining);

        static Ray ray_for_pixel(const Camera &camera, double px, double py);

        [[nodiscard]] bool is_shadowed(Point point) const;

        static Colour refracted_colour(const World &world, const Computations &comps, uint8_t remaining = 255);

        static Colour reflected_colour(const World &world, const Computations &comps, uint8_t remaining = 255);

        static float schlick(const Computations &comps);
    };
}

#endif //THE_RAYTRACER_CHALLENGE_WORLD_HPP