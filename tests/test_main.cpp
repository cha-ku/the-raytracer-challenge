//
// Created by chaku on 18/05/25.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "Matrix.hpp"
#include "Vector.hpp"
#include "Point.hpp"
#include "Colour.hpp"
#include "Utils.hpp"

using namespace raytracer;

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
    REQUIRE(utils::equal(Vector::magnitude({1, 2, 3}), std::sqrt(14)));
    REQUIRE(utils::equal(Vector::magnitude({-1, -2, -3}), std::sqrt(14)));
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

TEST_CASE("Create Container from range") {
    std::vector<double> tmp_vec{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    Container<double> container_vec(3, 3, tmp_vec);
    REQUIRE(std::ranges::equal(container_vec.m_data, tmp_vec));

    std::array<int, 9> tmp_array{1, 2, 3, 4, 5, 6, 7, 8, 9};
    Container<int> container_array(3, 3, tmp_array);
    REQUIRE(std::ranges::equal(container_array.m_data, tmp_array));

    const char tmp_char_array[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    Container<char> container_char(3, 3, tmp_char_array);
    REQUIRE(std::ranges::equal(container_char.m_data, tmp_char_array));
}

TEST_CASE("Container throws on invalid size") {
    std::array<double, 9> tmp{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    REQUIRE_THROWS_AS(Container<double>(4, 3, tmp), std::invalid_argument);
}

TEST_CASE("Matrix constructor test") {
    constexpr auto r = 2;
    constexpr auto c = 3;
    Container<int> container{r, c};
    REQUIRE(container.m_data == decltype(container.m_data)(2 * 3, 0));
    REQUIRE(container.m_rows == 2);
    REQUIRE(container.m_cols == 3);

    Matrix matrix{make_matrix(container)};
    REQUIRE(matrix[0, 0] == 0);
    matrix[1, 1] = 42;
    REQUIRE(matrix[1, 1] == 42);

    std::vector<double> tmp{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    REQUIRE(tmp.size() == 9);
    Container<double> container1(3, 3, tmp);
    Matrix matrix1{make_matrix(container1)};
    REQUIRE(matrix1[0, 0] == 1.0);
    REQUIRE(matrix1[0, 1] == 2.0);
    REQUIRE(matrix1[0, 2] == 3.0);
    REQUIRE(matrix1[1, 0] == 4.0);
    REQUIRE(matrix1[1, 1] == 5.0);
    REQUIRE(matrix1[1, 2] == 6.0);
    REQUIRE(matrix1[2, 0] == 7.0);
    REQUIRE(matrix1[2, 1] == 8.0);
    REQUIRE(matrix1[2, 2] == 9.0);
}


TEST_CASE("Matrix comparison test") {
    constexpr auto r = 3;
    constexpr auto c = 3;
    Container<int> container1{r, c, std::vector{1, 3, 5, 5, 6, 8, 9, 10, 32}};
    Matrix matrix1{make_matrix(container1)};
}
