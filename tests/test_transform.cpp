#include "MatrixImpl.hpp"
#include "Point.hpp"
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


