//
// Created by chaku on 18/05/25.
//

#define CATCH_CONFIG_MAIN
#include "MatrixImpl.hpp"
#include "Vector.hpp"
#include "Point.hpp"
#include "Colour.hpp"
#include "Utils.hpp"

#include <catch2/catch_all.hpp>

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
    constexpr Colour colour{0.2, 0.3, 0.4};
    REQUIRE(colour * 2.0 == Colour(0.4, 0.6, 0.8));
}

TEST_CASE("Create Container from range") {
    std::vector<double> tmp_vec{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    Container<double> container_vec(3, 3, tmp_vec);
    REQUIRE(std::ranges::equal(container_vec.m_data, tmp_vec));

    std::array<int, 9> tmp_array{1, 2, 3, 4, 5, 6, 7, 8, 9};
    Container<int> container_array(3, 3, tmp_array);
    REQUIRE(std::ranges::equal(container_array.m_data, tmp_array));

    constexpr char tmp_char_array[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
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

TEST_CASE("Matrix multiplication test") {
    constexpr auto dim = 4;
    Container<int> container1{dim, dim, std::vector{1, 2, 3, 4,
                                                                          5, 6, 7, 8,
                                                                          9, 8, 7, 6,
                                                                          5, 4, 3, 2}};

    Container<int> container2{dim, dim, std::vector{-2, 1, 2, 3,
                                                                         3, 2, 1, -1,
                                                                         4, 3, 6, 5,
                                                                         1, 2, 7, 8}};
    Container result{multiply(container1, container2)};
    Matrix result_mat{make_matrix(result)};
    REQUIRE(result_mat[0, 0] == 20);
    REQUIRE(result_mat[0, 1] == 22);
    REQUIRE(result_mat[0, 2] == 50);
    REQUIRE(result_mat[0, 3] == 48);
    REQUIRE(result_mat[1, 0] == 44);
    REQUIRE(result_mat[1, 1] == 54);
    REQUIRE(result_mat[1, 2] == 114);
    REQUIRE(result_mat[1, 3] == 108);
    REQUIRE(result_mat[2, 0] == 40);
    REQUIRE(result_mat[2, 1] == 58);
    REQUIRE(result_mat[2, 2] == 110);
    REQUIRE(result_mat[2, 3] == 102);
    REQUIRE(result_mat[3, 0] == 16);
    REQUIRE(result_mat[3, 1] == 26);
    REQUIRE(result_mat[3, 2] == 46);
    REQUIRE(result_mat[3, 3] == 42);

    Container<int> container3{dim, dim, std::vector{1, 2, 3, 4,
                                                    2, 4, 4, 2,
                                                    8, 6, 4, 1,
                                                    0, 0, 0, 1}};

    constexpr auto num_cols = 1;
    Container<int> container4{dim, num_cols, std::vector{1, 2, 3, 1}};
    Container result1{multiply(container3, container4)};
    Matrix result_mat1{make_matrix(result1)};
    REQUIRE(result_mat1[0, 0] == 18);
    REQUIRE(result_mat1[0, 1] == 24);
    REQUIRE(result_mat1[0, 2] == 33);
    REQUIRE(result_mat1[0, 3] == 1);
}

TEST_CASE("Identity matrix test") {
    Container<int> ident_mat{Container<int>::identity(3)};
    Matrix result_ident_mat{make_matrix(ident_mat)};
    REQUIRE(result_ident_mat[0, 0] == 1);
    REQUIRE(result_ident_mat[0, 1] == 0);
    REQUIRE(result_ident_mat[0, 2] == 0);
    REQUIRE(result_ident_mat[1, 0] == 0);
    REQUIRE(result_ident_mat[1, 1] == 1);
    REQUIRE(result_ident_mat[1, 2] == 0);
    REQUIRE(result_ident_mat[2, 0] == 0);
    REQUIRE(result_ident_mat[2, 1] == 0);
    REQUIRE(result_ident_mat[2, 2] == 1);
}

TEST_CASE("Matrix transpose test") {
    Container<int> transpose_test{4, 4};
    transpose_test.m_data = {0, 9, 3, 0,
                             9, 8, 0, 8,
                             1, 8, 5, 3,
                             0, 0, 5, 8};
    auto transposed{transpose(transpose_test)};
    Matrix transpose_mat{make_matrix(transposed)};
    REQUIRE(transpose_mat[0, 0] == 0);
    REQUIRE(transpose_mat[0, 1] == 9);
    REQUIRE(transpose_mat[0, 2] == 1);
    REQUIRE(transpose_mat[0, 3] == 0);
    REQUIRE(transpose_mat[1, 0] == 9);
    REQUIRE(transpose_mat[1, 1] == 8);
    REQUIRE(transpose_mat[1, 2] == 8);
    REQUIRE(transpose_mat[1, 3] == 0);
    REQUIRE(transpose_mat[2, 0] == 3);
    REQUIRE(transpose_mat[2, 1] == 0);
    REQUIRE(transpose_mat[2, 2] == 5);
    REQUIRE(transpose_mat[2, 3] == 5);
    REQUIRE(transpose_mat[3, 0] == 0);
    REQUIRE(transpose_mat[3, 1] == 8);
    REQUIRE(transpose_mat[3, 2] == 3);
    REQUIRE(transpose_mat[3, 3] == 8);
}

TEST_CASE("Submatrix test 1") {
    Container<int> submatrix_test{3, 3};
    submatrix_test.m_data = {1, 5, 0,
                            -3, 2, 7,
                             0, 6, -3};
    decltype(submatrix_test) minor{submatrix(submatrix_test, 0, 2)};
    REQUIRE(minor.m_data == decltype(submatrix_test.m_data){-3, 2, 0, 6});
    REQUIRE(minor.m_rows == submatrix_test.m_rows - 1);
    REQUIRE(minor.m_cols == submatrix_test.m_cols - 1);
}

TEST_CASE("Submatrix test 2") {
    Container<int> submatrix_test{4, 4};
    submatrix_test.m_data = {-6 , 1 , 1 , 6,
                             -8 , 5 , 8 , 6,
                             -1 , 0 , 8 , 2,
                             -7 , 1 , -1 , 1};
    decltype(submatrix_test) minor{submatrix(submatrix_test, 2, 1)};
    REQUIRE(minor.m_data == decltype(submatrix_test.m_data){
        -6 , 1 , 6,
        -8 , 8 , 6,
        -7 , -1 , 1});
    REQUIRE(minor.m_rows == submatrix_test.m_rows - 1);
    REQUIRE(minor.m_cols == submatrix_test.m_cols - 1);
}

TEST_CASE("Determinant test") {
    const Container<int> determinant_test2x2{2, 2, std::vector{1, 5, -3, 2}};
    REQUIRE(determinant(determinant_test2x2) == 17);
    const Container<int> bad_determinant_test{1, 2, std::vector{1, 5}};
    REQUIRE_THROWS_AS(determinant(bad_determinant_test), std::invalid_argument);
    const Container<int> determinant_test3x3{3, 3, std::vector{1, 2, 6, -5, 8, -4, 2, 6, 4}};
    REQUIRE(determinant(determinant_test3x3) == -196);
    const Container<int> determinant_test4x4{4, 4,
        std::vector{-2, -8, 3, 5,
                    -3, 1, 7, 3,
                    1, 2, -9, 6,
                    -6, 7, 7, -9}};
    REQUIRE(determinant(determinant_test4x4) == -4071);
}

TEST_CASE("Minor test") {
    const Container<int> minor_test{3, 3, std::vector{3, 5, 0, 2, -1, -7, 6, -1, 5}};
    REQUIRE(minor(minor_test, 1, 0) == 25);
}

TEST_CASE("Cofactor test") {
    const Container<int> cofactor_test{3, 3, std::vector{3, 5, 0, 2, -1, -7, 6, -1, 5}};
    REQUIRE(cofactor(cofactor_test, 0, 0) == -12);
    REQUIRE(cofactor(cofactor_test, 1, 0) == -25);
}
