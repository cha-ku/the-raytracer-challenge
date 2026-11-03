#include "Pattern.hpp"
#include "Colour.hpp"
#include "Point.hpp"
#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "TestPattern.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

// Background
constexpr Colour black{.r = 0, .g = 0, .b = 0};
constexpr Colour white{.r = 1, .g = 1, .b = 1};

SCENARIO("Creating a stripe pattern") {
    GIVEN("pattern ← stripe_pattern(white, black)") {
        const auto pattern = StripedPattern::create(white, black);

        THEN("pattern.a = white") {
            REQUIRE(areAlmostEqual(pattern.m_colour_a, white));
        }
        AND_THEN("pattern.b = black") {
            REQUIRE(areAlmostEqual(pattern.m_colour_b, black));
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

SCENARIO("The default pattern transformation") {
    GIVEN("pattern ← test_pattern()") {
        const auto pattern = test_pattern();

        THEN("pattern.transform = identity_matrix") {
            REQUIRE(pattern.m_transform == Matrix<double>::identity(4));
        }
    }
}

// "Assigning a transformation" in patterns.feature; renamed because Catch2 test names must be unique and
// test_shapes.cpp already has a scenario with that name
SCENARIO("Assigning a pattern transformation") {
    GIVEN("pattern ← test_pattern()") {
        auto pattern = test_pattern();

        WHEN("set_pattern_transform(pattern, translation(1, 2, 3))") {
            pattern.m_transform = translation<double>(1, 2, 3);

            THEN("pattern.transform = translation(1, 2, 3)") {
                REQUIRE(pattern.m_transform == translation<double>(1, 2, 3));
            }
        }
    }
}

SCENARIO("A pattern with an object transformation") {
    GIVEN("shape ← sphere(), set_transform(shape, scaling(2, 2, 2)) and pattern ← test_pattern()") {
        Sphere shape = Sphere::make_sphere();
        shape.set_transform(scale<double>(2, 2, 2));
        const auto pattern = test_pattern();

        WHEN("c ← pattern_at_shape(pattern, shape, point(2, 3, 4))") {
            const auto c = pattern_at_shape(pattern, shape.m_transform, Point{.x = 2, .y = 3, .z = 4});

            THEN("c = color(1, 1.5, 2)") {
                REQUIRE(areAlmostEqual(c, Colour{.r = 1, .g = 1.5, .b = 2}));
            }
        }
    }
}

SCENARIO("A pattern with a pattern transformation") {
    GIVEN("shape ← sphere(), pattern ← test_pattern() and set_pattern_transform(pattern, scaling(2, 2, 2))") {
        const Sphere shape = Sphere::make_sphere();
        auto pattern = test_pattern();
        pattern.m_transform = scale<double>(2, 2, 2);

        WHEN("c ← pattern_at_shape(pattern, shape, point(2, 3, 4))") {
            const auto c = pattern_at_shape(pattern, shape.m_transform, Point{.x = 2, .y = 3, .z = 4});

            THEN("c = color(1, 1.5, 2)") {
                REQUIRE(areAlmostEqual(c, Colour{.r = 1, .g = 1.5, .b = 2}));
            }
        }
    }
}

SCENARIO("A pattern with both an object and a pattern transformation") {
    GIVEN("shape ← sphere(), set_transform(shape, scaling(2, 2, 2)), pattern ← test_pattern() "
          "and set_pattern_transform(pattern, translation(0.5, 1, 1.5))") {
        Sphere shape = Sphere::make_sphere();
        shape.set_transform(scale<double>(2, 2, 2));
        auto pattern = test_pattern();
        pattern.m_transform = translation<double>(0.5, 1, 1.5);

        WHEN("c ← pattern_at_shape(pattern, shape, point(2.5, 3, 3.5))") {
            const auto c = pattern_at_shape(pattern, shape.m_transform, Point{.x = 2.5f, .y = 3, .z = 3.5f});

            THEN("c = color(0.75, 0.5, 0.25)") {
                REQUIRE(areAlmostEqual(c, Colour{.r = 0.75, .g = 0.5, .b = 0.25}));
            }
        }
    }
}
