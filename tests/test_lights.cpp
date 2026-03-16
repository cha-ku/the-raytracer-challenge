//
// Created by claude on 28/02/2026.
//

#include "Light.hpp"
#include "Point.hpp"
#include "Colour.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

SCENARIO("A point light has a position and intensity") {
    GIVEN("An intensity and a position") {
        constexpr Colour intensity{1, 1, 1};
        constexpr Point position{0, 0, 0};
        WHEN("light is created") {
            const PointLight light{position, intensity};
            THEN("light has the given position and intensity") {
                REQUIRE(light.position == position);
                REQUIRE(light.intensity == intensity);
            }
        }
    }
}
