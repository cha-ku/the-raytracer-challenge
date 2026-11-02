#include "Pattern.hpp"
#include "Colour.hpp"
#include "Point.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

// Background
constexpr Colour black{.r = 0, .g = 0, .b = 0};
constexpr Colour white{.r = 1, .g = 1, .b = 1};

SCENARIO("Creating a stripe pattern") {
    GIVEN("pattern ← stripe_pattern(white, black)") {
        const auto pattern = StripedPattern::create(white, black);

        THEN("pattern.a = white") {
            REQUIRE(areAlmostEqual(pattern.a, white));
        }
        AND_THEN("pattern.b = black") {
            REQUIRE(areAlmostEqual(pattern.b, black));
        }
    }
}

SCENARIO("A stripe pattern is constant in y") {
    GIVEN("pattern ← stripe_pattern(white, black)") {
        const auto pattern = StripedPattern::create(white, black);

        THEN("stripe_at(pattern, point(0, 0, 0)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 0, .y = 0, .z = 0}), white));
        }
        AND_THEN("stripe_at(pattern, point(0, 1, 0)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 0, .y = 1, .z = 0}), white));
        }
        AND_THEN("stripe_at(pattern, point(0, 2, 0)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 0, .y = 2, .z = 0}), white));
        }
    }
}

SCENARIO("A stripe pattern is constant in z") {
    GIVEN("pattern ← stripe_pattern(white, black)") {
        const auto pattern = StripedPattern::create(white, black);

        THEN("stripe_at(pattern, point(0, 0, 0)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 0, .y = 0, .z = 0}), white));
        }
        AND_THEN("stripe_at(pattern, point(0, 0, 1)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 0, .y = 0, .z = 1}), white));
        }
        AND_THEN("stripe_at(pattern, point(0, 0, 2)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 0, .y = 0, .z = 2}), white));
        }
    }
}

SCENARIO("A stripe pattern alternates in x") {
    GIVEN("pattern ← stripe_pattern(white, black)") {
        const auto pattern = StripedPattern::create(white, black);

        THEN("stripe_at(pattern, point(0, 0, 0)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 0, .y = 0, .z = 0}), white));
        }
        AND_THEN("stripe_at(pattern, point(0.9, 0, 0)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 0.9f, .y = 0, .z = 0}), white));
        }
        AND_THEN("stripe_at(pattern, point(1, 0, 0)) = black") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = 1, .y = 0, .z = 0}), black));
        }
        AND_THEN("stripe_at(pattern, point(-0.1, 0, 0)) = black") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = -0.1f, .y = 0, .z = 0}), black));
        }
        AND_THEN("stripe_at(pattern, point(-1, 0, 0)) = black") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = -1, .y = 0, .z = 0}), black));
        }
        AND_THEN("stripe_at(pattern, point(-1.1, 0, 0)) = white") {
            REQUIRE(areAlmostEqual(StripedPattern::stripe_at(pattern, Point{.x = -1.1f, .y = 0, .z = 0}), white));
        }
    }
}
