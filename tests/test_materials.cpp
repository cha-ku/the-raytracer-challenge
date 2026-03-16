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

TEST_CASE("Materials") {
    // Background
    constexpr Material m{};
    constexpr Point position{0, 0, 0};

    SECTION("The default material") {
        REQUIRE(areAlmostEqual(m.colour, Colour{1, 1, 1}));
        REQUIRE(m.ambient == 0.1f);
        REQUIRE(m.diffuse == 0.9f);
        REQUIRE(m.specular == 0.9f);
        REQUIRE(m.shininess == 200.0f);
    }

    SECTION("Lighting with the eye between the light and the surface") {
        constexpr Vector eye{0, 0, -1};
        constexpr Vector normal{0, 0, -1};
        constexpr PointLight light{Point{0, 0, -10}, Colour{1, 1, 1}};
        const auto result = lighting(m, light, position, eye, normal);
        REQUIRE(areAlmostEqual(result, Colour{1.9, 1.9, 1.9}));
    }

    SECTION("Lighting with the eye between light and surface, eye offset 45°") {
        constexpr Vector eye{0, std::numbers::sqrt2 / 2, -std::numbers::sqrt2 / 2};
        constexpr Vector normal{0, 0, -1};
        constexpr PointLight light{Point{0, 0, -10}, Colour{1, 1, 1}};
        const auto result = lighting(m, light, position, eye, normal);
        REQUIRE(areAlmostEqual(result, Colour{1.0, 1.0, 1.0}));
    }

    SECTION("Lighting with the light behind the surface") {
        constexpr Vector eye{0, 0, -1};
        constexpr Vector normal{0, 0, -1};
        constexpr PointLight light{Point{0, 0, 10}, Colour{1, 1, 1}};
        const auto result = lighting(m, light, position, eye, normal);
        REQUIRE(areAlmostEqual(result, Colour{0.1, 0.1, 0.1}));
    }
}
