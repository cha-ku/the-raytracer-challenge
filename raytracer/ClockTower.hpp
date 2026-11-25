#ifndef THE_RAYTRACER_CHALLENGE_CLOCKTOWER_HPP
#define THE_RAYTRACER_CHALLENGE_CLOCKTOWER_HPP

#include "World.hpp"

namespace raytracer {
    // Adds a CSG model of the Elizabeth Tower (Big Ben) to `world` and returns the group that holds it. It stands
    // on y = 0, centred on the y axis, about 1 unit per 10 metres: 11.5 tall and 1.6 wide at its widest.
    Geom &add_clock_tower(World &world);
}

#endif //THE_RAYTRACER_CHALLENGE_CLOCKTOWER_HPP
