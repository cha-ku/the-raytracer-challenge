//
// Created by chaku on 28/02/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_LIGHT_HPP
#define THE_RAYTRACER_CHALLENGE_LIGHT_HPP

#include "Colour.hpp"
#include "Point.hpp"

namespace raytracer {
    struct PointLight {
        Point position;
        Colour intensity{0, 0, 0};
    };
}

#endif //THE_RAYTRACER_CHALLENGE_LIGHT_HPP