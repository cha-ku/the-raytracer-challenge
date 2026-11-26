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

#include <array>
#include <type_traits>

using namespace raytracer;

TEST_CASE (
"Vector addition test"
)
 {
    REQUIRE(Vector(1, 2, 3) + Vector(4, 5, 6) == Vector(5, 7, 9));
}

TEST_CASE (
"Point addition test"
)
 {
    REQUIRE(Point(1, 2, 3) + Point(4, 5, 6) == Point(5, 7, 9));
}

TEST_CASE (
"Subtracting two points"
)
 {
    REQUIRE(Point(3, 2, 1) - Point(5, 6, 7) == Vector(-2, -4, -6));
}

TEST_CASE (
"Subtracting a vector from a point"
)
 {
    REQUIRE(Point(3, 2, 1) - Vector(5, 6, 7) == Point(-2, -4, -6));
}

TEST_CASE (
"Subtracting two vectors"
)
 {
    REQUIRE(Vector(3, 2, 1) - Vector(5, 6, 7) == Vector(-2, -4, -6));
}

TEST_CASE (
"Negate a vector"
)
 {
    REQUIRE(-Vector(1, -2, 3) == Vector(-1, 2, -3));
}

TEST_CASE (
"Multiply a vector by a scalar"
)
 {
    REQUIRE(Vector(1, -2, 3) * 3.5 == Vector(3.5, -7, 10.5));
}

TEST_CASE (
"Divide a vector by a scalar"
)
 {
    REQUIRE(Vector(1, -2, 3) / 2 == Vector(0.5, -1, 1.5));
}

TEST_CASE (
"Magnitude of a vector"
)
 {
    REQUIRE(Vector::magnitude({1, 0, 0}) == 1);
    REQUIRE(Vector::magnitude({0, 1, 0}) == 1);
    REQUIRE(Vector::magnitude({0, 0, 1}) == 1);
    REQUIRE(utils::equal(Vector::magnitude({1, 2, 3}), std::sqrt(14)));
    REQUIRE(utils::equal(Vector::magnitude({-1, -2, -3}), std::sqrt(14)));
}

TEST_CASE (
"Normalization of a vector"
)
 {
    REQUIRE(Vector::normalize({1, 0, 0}) == Vector(1, 0, 0));
    REQUIRE(Vector::normalize({4, 0, 0}) == Vector(1, 0, 0));
    static const auto tmp = std::sqrt(14);
    REQUIRE(Vector::areAlmostEqual(Vector::normalize({1, 2, 3}), Vector(1.0/tmp, 2.0/tmp, 3.0/tmp)));
}

TEST_CASE (
"Dot product"
)
 {
    REQUIRE(Vector::dot({1, 2, 3} , {2, 3, 4}) == 20);
}

TEST_CASE (
"Cross product"
)
 {
    REQUIRE(Vector::cross({1, 2, 3}, {2, 3, 4}) == Vector{-1, 2, -1});
    REQUIRE(Vector::cross({2, 3, 4}, {1, 2, 3}) == Vector{1, -2, 1});
}

TEST_CASE (
"Colour addition test"
)
 {
    REQUIRE(areAlmostEqual(Colour(0.9, 0.6, 0.75) + Colour(0.7, 0.1, 0.25), Colour(1.6, 0.7, 1.0)));
}

TEST_CASE (
"Colour subtraction test"
)
 {
    REQUIRE(areAlmostEqual(Colour(0.9, 0.6, 0.75) - Colour(0.7, 0.1, 0.25), Colour(0.2, 0.5, 0.5)));
}

TEST_CASE (
"Colour multiplication test"
)
 {
    constexpr Colour colour{0.2, 0.3, 0.4};
    REQUIRE(colour * 2.0 == Colour(0.4, 0.6, 0.8));
}

namespace {
    // Whether determinant() can be called on a matrix of type M (only square matrices of size 2 or more)
    template<typename M>
    concept HasDeterminant = requires(const M &matrix) { determinant(matrix); };
}

TEST_CASE (
"Create Matrix from range"
)
 {
    constexpr std::array tmp_doubles{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    const Matrix<double, 3, 3> matrix_doubles{tmp_doubles};
    REQUIRE(std::ranges::equal(matrix_doubles.data(), tmp_doubles));

    constexpr std::array<int, 9> tmp_array{1, 2, 3, 4, 5, 6, 7, 8, 9};
    const Matrix<int, 3, 3> matrix_array{tmp_array};
    REQUIRE(std::ranges::equal(matrix_array.data(), tmp_array));

    constexpr char tmp_char_array[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    const Matrix<char, 3, 3> matrix_char{std::to_array(tmp_char_array)};
    REQUIRE(std::ranges::equal(matrix_char.data(), tmp_char_array));
}

TEST_CASE (
"Matrix rejects data of the wrong size at compile time"
)
 {
    using Matrix4x3 = Matrix<double, 4, 3>;
    STATIC_REQUIRE_FALSE(std::is_constructible_v<Matrix4x3, std::array<double, 9>>);
    STATIC_REQUIRE(std::is_constructible_v<Matrix4x3, std::array<double, 12>>);
}

TEST_CASE (
"Matrix constructor test"
)
 {
    Matrix<int, 2, 3> matrix;
    REQUIRE(matrix.data() == std::array<int, 6>{});
    STATIC_REQUIRE(Matrix<int, 2, 3>::rows() == 2);
    STATIC_REQUIRE(Matrix<int, 2, 3>::cols() == 3);

    REQUIRE(matrix[0, 0] == 0);
    matrix[1, 1] = 42;
    REQUIRE(matrix[1, 1] == 42);

    const Matrix<double, 3, 3> matrix1{{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0}};
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


TEST_CASE (
"Matrix comparison test"
)
 {
    const Matrix<int, 3, 3> matrix1{{1, 3, 5, 5, 6, 8, 9, 10, 32}};
    const Matrix<int, 3, 3> same{{1, 3, 5, 5, 6, 8, 9, 10, 32}};
    const Matrix<int, 3, 3> different{{1, 3, 5, 5, 6, 8, 9, 10, 33}};
    REQUIRE(matrix1 == same);
    REQUIRE_FALSE(matrix1 == different);
}

TEST_CASE (
"Matrix multiplication test"
)
 {
    const Matrix<int> matrix1{
        {
            1, 2, 3, 4,
            5, 6, 7, 8,
            9, 8, 7, 6,
            5, 4, 3, 2
        }
    };

    const Matrix<int> matrix2{
        {
            -2, 1, 2, 3,
            3, 2, 1, -1,
            4, 3, 6, 5,
            1, 2, 7, 8
        }
    };
    const Matrix result{multiply(matrix1, matrix2)};
    REQUIRE(result[0, 0] == 20);
    REQUIRE(result[0, 1] == 22);
    REQUIRE(result[0, 2] == 50);
    REQUIRE(result[0, 3] == 48);
    REQUIRE(result[1, 0] == 44);
    REQUIRE(result[1, 1] == 54);
    REQUIRE(result[1, 2] == 114);
    REQUIRE(result[1, 3] == 108);
    REQUIRE(result[2, 0] == 40);
    REQUIRE(result[2, 1] == 58);
    REQUIRE(result[2, 2] == 110);
    REQUIRE(result[2, 3] == 102);
    REQUIRE(result[3, 0] == 16);
    REQUIRE(result[3, 1] == 26);
    REQUIRE(result[3, 2] == 46);
    REQUIRE(result[3, 3] == 42);

    const Matrix<int> matrix3{
        {
            1, 2, 3, 4,
            2, 4, 4, 2,
            8, 6, 4, 1,
            0, 0, 0, 1
        }
    };

    const Matrix<int, 4, 1> matrix4{{1, 2, 3, 1}};
    const Matrix result1{multiply(matrix3, matrix4)};
    REQUIRE(result1[0, 0] == 18);
    REQUIRE(result1[1, 0] == 24);
    REQUIRE(result1[2, 0] == 33);
    REQUIRE(result1[3, 0] == 1);
}

TEST_CASE (
"Identity matrix test"
)
 {
    const Matrix ident_mat{Matrix<int, 3, 3>::identity()};
    REQUIRE(ident_mat[0, 0] == 1);
    REQUIRE(ident_mat[0, 1] == 0);
    REQUIRE(ident_mat[0, 2] == 0);
    REQUIRE(ident_mat[1, 0] == 0);
    REQUIRE(ident_mat[1, 1] == 1);
    REQUIRE(ident_mat[1, 2] == 0);
    REQUIRE(ident_mat[2, 0] == 0);
    REQUIRE(ident_mat[2, 1] == 0);
    REQUIRE(ident_mat[2, 2] == 1);
}

TEST_CASE (
"Matrix transpose test"
)
 {
    const Matrix<int> transpose_test{{
        0, 9, 3, 0,
        9, 8, 0, 8,
        1, 8, 5, 3,
        0, 0, 5, 8
    }};
    const auto transpose_mat{transpose(transpose_test)};
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

TEST_CASE (
"Submatrix test 1"
)
 {
    const Matrix<int, 3, 3> submatrix_test{{
        1, 5, 0,
        -3, 2, 7,
        0, 6, -3
    }};
    const auto minor{submatrix(submatrix_test, 0, 2)};
    REQUIRE(minor.data() == std::array{-3, 2, 0, 6});
    STATIC_REQUIRE(decltype(minor)::rows() == decltype(submatrix_test)::rows() - 1);
    STATIC_REQUIRE(decltype(minor)::cols() == decltype(submatrix_test)::cols() - 1);
}

TEST_CASE (
"Submatrix test 2"
)
 {
    const Matrix<int> submatrix_test{{
        -6, 1, 1, 6,
        -8, 5, 8, 6,
        -1, 0, 8, 2,
        -7, 1, -1, 1
    }};
    const auto minor{submatrix(submatrix_test, 2, 1)};
    REQUIRE(minor.data() == std::array{
            -6, 1, 6,
            -8, 8, 6,
            -7, -1, 1});
    STATIC_REQUIRE(decltype(minor)::rows() == decltype(submatrix_test)::rows() - 1);
    STATIC_REQUIRE(decltype(minor)::cols() == decltype(submatrix_test)::cols() - 1);
}

TEST_CASE (
"Determinant test"
)
 {
    const Matrix<int, 2, 2> determinant_test2x2{{1, 5, -3, 2}};
    REQUIRE(determinant(determinant_test2x2) == 17);
    // Only square matrices of size 2 or more have a determinant; anything else doesn't compile
    STATIC_REQUIRE_FALSE(HasDeterminant<Matrix<int, 1, 2>>);
    STATIC_REQUIRE_FALSE(HasDeterminant<Matrix<int, 1, 1>>);
    STATIC_REQUIRE(HasDeterminant<Matrix<int, 2, 2>>);
    const Matrix<int, 3, 3> determinant_test3x3{{1, 2, 6, -5, 8, -4, 2, 6, 4}};
    REQUIRE(determinant(determinant_test3x3) == -196);
    const Matrix<int> determinant_test4x4{
        {
            -2, -8, 3, 5,
            -3, 1, 7, 3,
            1, 2, -9, 6,
            -6, 7, 7, -9
        }
    };
    REQUIRE(determinant(determinant_test4x4) == -4071);
}

TEST_CASE (
"Minor test"
)
 {
    const Matrix<int, 3, 3> minor_test{{3, 5, 0, 2, -1, -7, 6, -1, 5}};
    REQUIRE(minor(minor_test, 1, 0) == 25);
}

TEST_CASE (
"Cofactor test"
)
 {
    const Matrix<int, 3, 3> cofactor_test{{3, 5, 0, 2, -1, -7, 6, -1, 5}};
    REQUIRE(cofactor(cofactor_test, 0, 0) == -12);
    REQUIRE(cofactor(cofactor_test, 1, 0) == -25);
}

TEST_CASE (
"Inverse test"
)
 {
    const Matrix<int> inverse_test{
        {
            -5, 2, 6, -8,
            1, -5, 1, 8,
            7, 7, -6, -7,
            1, -3, 7, 4
        }
    };

    REQUIRE(inverse(inverse_test).value() == Matrix<double>{{
            0.21805, 0.45113, 0.24060, -0.04511,
            -0.80827, -1.45677, -0.44361, 0.52068,
            -0.07895, -0.22368, -0.05263, 0.19737,
            -0.52256, -0.81391, -0.30075, 0.30639 }});

    const Matrix<double> a{
        {
            3, -9, 7, 3,
            3, -8, 2, -9,
            -4, 4, 4, 1,
            -6, 5, -1, 1
        }
    };

    const Matrix<double> b{
        {
            8, 2, 2, 2,
            3, -1, 7, 0,
            7, 0, 5, 4,
            6, -2, 0, 5
        }
    };

    const Matrix c{multiply(a, b)};

    REQUIRE(multiply(c, inverse(b).value()) == a);
}
