#include "MatrixImpl.hpp"
#include "Point.hpp"
#include "Intersect.hpp"
#include <catch2/catch_all.hpp>
#include <iostream>
#include <numbers>

using namespace raytracer;

TEST_CASE("Translation test") {
    const auto transform = translation<double>(5, -3, 2);
    const auto point = make_container(Point(-3, 4, 5));

    SECTION("Multiplying by a translation matrix") {
        const Container product{multiply(transform, point)};
        REQUIRE(product == make_container(Point{2, 1, 7}));
    }

    SECTION("Multiplying by the inverse of a translation matrix") {
        const auto transform_inverse = inverse(transform).value();
        REQUIRE(multiply(transform_inverse, point) == make_container({-8, 7, 3 }));
    }
}

TEST_CASE("Scale test") {
    const auto transform = scale<double>(2, 3, 4);
    const auto point = make_container(Point(-4, 6, 8));
    SECTION("A scaling matrix applied to a point") {
        REQUIRE(multiply(transform, point) == make_container(Point{-8, 18, 32}));
    }

    SECTION("Multiplying by the inverse of a scaling matrix") {
        const auto transform_inverse{inverse(transform).value()};
        REQUIRE(multiply(transform_inverse, point) == make_container(Point{-2, 2, 2}));
    }

    SECTION("Reflection is scaling by a negative value") {
        const auto& x_reflection_transform = scale<double>(-1, 1, 1);
        const auto& point_to_reflect = make_container(Point{2, 3, 4});
        REQUIRE(multiply(x_reflection_transform, point_to_reflect) == make_container(Point{-2, 3, 4}));
    }
}

TEST_CASE("Rotation test") {
    using std::numbers::pi;
    using std::numbers::sqrt2;
    SECTION("Rotating a point around the x axis") {
        const auto point{make_container(Point(0, 1, 0))};
        const auto half_quarter{rotation_x(pi/4)};
        const auto full_quarter{rotation_x(pi/2)};
        REQUIRE(multiply(half_quarter, point) ==
            make_container(Point{0, sqrt2/2, sqrt2/2}));
        REQUIRE(multiply(full_quarter, point) ==
            make_container(Point{0, 0, 1}));
        const auto inverse_half_quarter{inverse(half_quarter).value()};
        REQUIRE(multiply(inverse_half_quarter, point) ==
            make_container(Point{0, sqrt2/2, -sqrt2/2}));
    }

    SECTION("Rotating a point around the y axis") {
        const auto point{make_container(Point(0, 0, 1))};
        const auto half_quarter{rotation_y(pi/4)};
        const auto full_quarter{rotation_y(pi/2)};
        REQUIRE(multiply(half_quarter, point) ==
            make_container(Point{sqrt2/2, 0, sqrt2/2}));
        REQUIRE(multiply(full_quarter, point) == make_container(Point{1, 0, 0}));
    }

    SECTION("Rotating a point around the z axis") {
        const auto point{make_container(Point(0, 1, 0))};
        const auto half_quarter{rotation_z(pi/4)};
        const auto full_quarter{rotation_z(pi/2)};
        REQUIRE(multiply(half_quarter, point) ==
            make_container(Point{-sqrt2/2,  sqrt2/2, 0}));
        REQUIRE(multiply(full_quarter, point) == make_container(Point{-1, 0, 0}));
    }
}

TEST_CASE("Shearing test") {
    SECTION("Shearing transformation moves x in proportion to z") {
        const auto transform{shearing(0, 1, 0, 0, 0, 0)};
        const auto point{make_container(Point(2, 3, 4))};
        REQUIRE(multiply(transform, point) == make_container(Point(6, 3, 4)));
    }

    SECTION("Shearing transformation moves y in proportion to x") {
        const auto transform{shearing(0, 0, 1, 0, 0, 0)};
        const auto point{make_container(Point(2, 3, 4))};
        REQUIRE(multiply(transform, point) == make_container(Point(2, 5, 4)));
    }

    SECTION("Shearing transformation moves y in proportion to z") {
        const auto transform{shearing(0, 0, 0, 1, 0, 0)};
        const auto point{make_container(Point(2, 3, 4))};
        REQUIRE(multiply(transform, point) == make_container(Point(2, 7, 4)));
    }

    SECTION("Shearing transformation moves z in proportion to x") {
        const auto transform{shearing(0, 0, 0, 0, 1, 0)};
        const auto point{make_container(Point(2, 3, 4))};
        REQUIRE(multiply(transform, point) == make_container(Point(2, 3, 6)));
    }

    SECTION("Shearing transformation moves z in proportion to y") {
        const auto transform{shearing(0, 0, 0, 0, 0, 1)};
        const auto point{make_container(Point(2, 3, 4))};
        REQUIRE(multiply(transform, point) == make_container(Point(2, 3, 7)));
    }
}

SCENARIO("The transformation matrix for the default orientation") {
    GIVEN("from ← point(0, 0, 0), to ← point(0, 0, -1), up ← vector(0, 1, 0)") {
        const Point from{0, 0, 0};
        const Point to{0, 0, -1};
        const Vector up{0, 1, 0};

        WHEN("t ← view_transform(from, to, up)") {
            const auto t = view_transform(from, to, up);

            THEN("t = identity_matrix") {
                REQUIRE(t == Container<double>::identity(4));
            }
        }
    }
}

SCENARIO("A view transformation matrix looking in positive z direction") {
    GIVEN("from ← point(0, 0, 0), to ← point(0, 0, 1), up ← vector(0, 1, 0)") {
        const Point from{0, 0, 0};
        const Point to{0, 0, 1};
        const Vector up{0, 1, 0};

        WHEN("t ← view_transform(from, to, up)") {
            const auto t = view_transform(from, to, up);

            THEN("t = scaling(-1, 1, -1)") {
                REQUIRE(t == scale<double>(-1, 1, -1));
            }
        }
    }
}

SCENARIO("The view transformation moves the world") {
    GIVEN("from ← point(0, 0, 8), to ← point(0, 0, 0), up ← vector(0, 1, 0)") {
        const Point from{0, 0, 8};
        const Point to{0, 0, 0};
        const Vector up{0, 1, 0};

        WHEN("t ← view_transform(from, to, up)") {
            const auto t = view_transform(from, to, up);

            THEN("t = translation(0, 0, -8)") {
                REQUIRE(t == translation<double>(0, 0, -8));
            }
        }
    }
}

SCENARIO("An arbitrary view transformation") {
    GIVEN("from ← point(1, 3, 2), to ← point(4, -2, 8), up ← vector(1, 1, 0)") {
        const Point from{1, 3, 2};
        const Point to{4, -2, 8};
        const Vector up{1, 1, 0};

        WHEN("t ← view_transform(from, to, up)") {
            const auto t = view_transform(from, to, up);

            THEN("t is the expected 4x4 matrix") {
                const Container<double> expected{4, 4,
                    std::array{-0.50709, 0.50709,  0.67612, -2.36643,
                      0.76772, 0.60609,  0.12122, -2.82843,
                     -0.35857, 0.59761, -0.71714,  0.00000,
                      0.00000, 0.00000,  0.00000,  1.00000
                    }
                };
                REQUIRE(t == expected);
            }
        }
    }
}


