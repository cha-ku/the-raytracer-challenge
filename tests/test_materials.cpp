//
// Created by claude on 28/02/2026.
//

#include "Material.hpp"
#include "Light.hpp"
#include "Colour.hpp"
#include "Point.hpp"
#include "Vector.hpp"

#include "catch2/catch_test_macros.hpp"

#include <numbers>

using namespace raytracer;

SCENARIO("The default material") {
    GIVEN("m ← material()") {
        constexpr Material m{};

        THEN("m.color = color(1, 1, 1)") {
            REQUIRE(areAlmostEqual(m.colour, Colour{1, 1, 1}));
        }
        AND_THEN("m.ambient = 0.1") {
            REQUIRE(m.ambient == 0.1f);
        }
        AND_THEN("m.diffuse = 0.9") {
            REQUIRE(m.diffuse == 0.9f);
        }
        AND_THEN("m.specular = 0.9") {
            REQUIRE(m.specular == 0.9f);
        }
        AND_THEN("m.shininess = 200.0") {
            REQUIRE(m.shininess == 200.0f);
        }
    }
}

SCENARIO("Lighting with the eye between the light and the surface") {
    GIVEN("m ← material() and position ← point(0, 0, 0)") {
        constexpr Material m{};
        constexpr Point position{0, 0, 0};
        constexpr Vector eye{0, 0, -1};
        constexpr Vector normal{0, 0, -1};
        constexpr PointLight light{Point{0, 0, -10}, Colour{1, 1, 1}};

        WHEN("result ← lighting(m, light, position, eyev, normalv)") {
            const auto result = lighting(m, light, position, eye, normal, false);

            THEN("result = color(1.9, 1.9, 1.9)") {
                REQUIRE(areAlmostEqual(result, Colour{1.9, 1.9, 1.9}));
            }
        }
    }
}

SCENARIO("Lighting with the eye between light and surface, eye offset 45 degrees") {
    GIVEN("m ← material() and position ← point(0, 0, 0)") {
        constexpr Material m{};
        constexpr Point position{0, 0, 0};
        constexpr Vector eye{0, std::numbers::sqrt2 / 2, -std::numbers::sqrt2 / 2};
        constexpr Vector normal{0, 0, -1};
        constexpr PointLight light{Point{0, 0, -10}, Colour{1, 1, 1}};

        WHEN("result ← lighting(m, light, position, eyev, normalv)") {
            const auto result = lighting(m, light, position, eye, normal, false);

            THEN("result = color(1.0, 1.0, 1.0)") {
                REQUIRE(areAlmostEqual(result, Colour{1.0, 1.0, 1.0}));
            }
        }
    }
}

SCENARIO("Lighting with the light behind the surface") {
    GIVEN("m ← material() and position ← point(0, 0, 0)") {
        constexpr Material m{};
        constexpr Point position{0, 0, 0};
        constexpr Vector eye{0, 0, -1};
        constexpr Vector normal{0, 0, -1};
        constexpr PointLight light{Point{0, 0, 10}, Colour{1, 1, 1}};

        WHEN("result ← lighting(m, light, position, eyev, normalv)") {
            const auto result = lighting(m, light, position, eye, normal, false);

            THEN("result = color(0.1, 0.1, 0.1)") {
                REQUIRE(areAlmostEqual(result, Colour{0.1, 0.1, 0.1}));
            }
        }
    }
}

SCENARIO("Lighting with the surface in shadow") {
    GIVEN("m ← material() and position ← point(0, 0, 0)") {
        constexpr Material m{};
        constexpr Point position{0, 0, 0};
        constexpr Vector eye{0, 0, -1};
        constexpr Vector normal{0, 0, -1};
        constexpr PointLight light{Point{0, 0, -10}, Colour{1, 1, 1}};
        constexpr bool in_shadow = true;

        WHEN("result ← lighting(m, light, position, eyev, normalv, in_shadow)") {
            const auto result = lighting(m, light, position, eye, normal, in_shadow);

            THEN("result = color(0.1, 0.1, 0.1)") {
                REQUIRE(areAlmostEqual(result, Colour{0.1, 0.1, 0.1}));
            }
        }
    }
}
