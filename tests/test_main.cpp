//
// Created by chaku on 18/05/25.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "Vector.hpp"
#include "Point.hpp"
#include "Colour.hpp"
#include "Utils.hpp"

TEST_CASE("Vector addition test") {
    REQUIRE(Vector(1, 2, 3) + Vector(4, 5, 6) == Vector(5, 7, 9));
}

TEST_CASE("Point addition test") {
    REQUIRE(Point(1, 2, 3) + Point(4, 5, 6) == Point(5, 7, 9));
}

TEST_CASE("Subtracting two points") {
    REQUIRE(Point(3, 2, 1) - Point(5, 6, 7) == Vector(-2, -4, -6));
}

TEST_CASE("Subtracting a vector from a point") {
    REQUIRE(Point(3, 2, 1) - Vector(5, 6, 7) == Point(-2, -4, -6));
}

TEST_CASE("Subtracting two vectors") {
    REQUIRE(Vector(3, 2, 1) - Vector(5, 6, 7) == Vector(-2, -4, -6));
}

TEST_CASE("Negate a vector") {
    REQUIRE(-Vector(1, -2, 3) == Vector(-1, 2, -3));
}

TEST_CASE("Multiply a vector by a scalar") {
    REQUIRE(Vector(1, -2, 3) * 3.5 == Vector(3.5, -7, 10.5));
}

TEST_CASE("Divide a vector by a scalar") {
    REQUIRE(Vector(1, -2, 3) / 2 == Vector(0.5, -1, 1.5));
}

TEST_CASE("Magnitude of a vector") {
    REQUIRE(Vector::magnitude({1, 0, 0}) == 1);
    REQUIRE(Vector::magnitude({0, 1, 0}) == 1);
    REQUIRE(Vector::magnitude({0, 0, 1}) == 1);
    REQUIRE(utils::areAlmostEqual(Vector::magnitude({1, 2, 3}), std::sqrt(14)));
    REQUIRE(utils::areAlmostEqual(Vector::magnitude({-1, -2, -3}), std::sqrt(14)));
}

TEST_CASE("Normalization of a vector") {
    REQUIRE(Vector::normalize({1, 0, 0}) == Vector(1, 0, 0));
    REQUIRE(Vector::normalize({4, 0, 0}) == Vector(1, 0, 0));
    static const auto tmp = std::sqrt(14);
    REQUIRE(Vector::areAlmostEqual(Vector::normalize({1, 2, 3}), Vector(1.0/tmp, 2.0/tmp, 3.0/tmp)));
}

TEST_CASE("Dot product") {
    REQUIRE(Vector::dot({1, 2, 3} , {2, 3, 4}) == 20);
}

TEST_CASE("Cross product") {
    REQUIRE(Vector::cross({1, 2, 3}, {2, 3, 4}) == Vector{-1, 2, -1});
    REQUIRE(Vector::cross({2, 3, 4}, {1, 2, 3}) == Vector{1, -2, 1});
}

TEST_CASE("Colour addition test") {
    REQUIRE(areAlmostEqual(Colour(0.9, 0.6, 0.75) + Colour(0.7, 0.1, 0.25), Colour(1.6, 0.7, 1.0)));
}

TEST_CASE("Colour subtraction test") {
    REQUIRE(areAlmostEqual(Colour(0.9, 0.6, 0.75) - Colour(0.7, 0.1, 0.25), Colour(0.2, 0.5, 0.5)));
}

TEST_CASE("Colour multiplication test") {
    const Colour colour{0.2, 0.3, 0.4};
    REQUIRE(colour * 2.0 == Colour(0.4, 0.6, 0.8));
}
