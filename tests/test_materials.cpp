//
// Created by claude on 28/02/2026.
//

#include "Material.hpp"
#include "Light.hpp"
#include "Colour.hpp"
#include "Pattern.hpp"
#include "Point.hpp"
#include "Vector.hpp"

#include "catch2/catch_test_macros.hpp"

#include <numbers>

using namespace raytracer;

SCENARIO("The default material") {
    GIVEN("m ← material()") {
        constexpr Material m{};

        THEN("m.color = color(1, 1, 1)") {
            REQUIRE(areAlmostEqual(std::get<Colour>(m.colour), Colour{1, 1, 1}));
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

SCENARIO("Lighting with a pattern applied") {
    GIVEN("m.pattern ← stripe_pattern(color(1, 1, 1), color(0, 0, 0))") {
        Material m{};
        m.colour = StripedPattern::create(Colour{.r = 1, .g = 1, .b = 1}, Colour{.r = 0, .g = 0, .b = 0});
        m.ambient = 1;
        m.diffuse = 0;
        m.specular = 0;
        constexpr Vector eye{.x = 0, .y = 0, .z = -1};
        constexpr Vector normal{.x = 0, .y = 0, .z = -1};
        constexpr PointLight light{.position = Point{.x = 0, .y = 0, .z = -10}, .intensity = Colour{.r = 1, .g = 1, .b = 1}};

        WHEN("c1 ← lighting(m, light, point(0.9, 0, 0), eyev, normalv, false) "
             "and c2 ← lighting(m, light, point(1.1, 0, 0), eyev, normalv, false)") {
            const auto c1 = lighting(m, light, Point{.x = 0.9f, .y = 0, .z = 0}, eye, normal, false);
            const auto c2 = lighting(m, light, Point{.x = 1.1f, .y = 0, .z = 0}, eye, normal, false);

            THEN("c1 = color(1, 1, 1)") {
                REQUIRE(areAlmostEqual(c1, Colour{.r = 1, .g = 1, .b = 1}));
            }
            AND_THEN("c2 = color(0, 0, 0)") {
                REQUIRE(areAlmostEqual(c2, Colour{.r = 0, .g = 0, .b = 0}));
            }
        }
    }
}
