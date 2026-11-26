//
// Created by chaku on 07/09/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATRIX_HPP
#define THE_RAYTRACER_CHALLENGE_MATRIX_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <type_traits>

#include "Point.hpp"
#include "Vector.hpp"
#include "Utils.hpp"

namespace raytracer {
    // A NUM_ROWS x NUM_COLS matrix stored row by row in a fixed-size array, so it never allocates. The size is part of
    // the type: mismatched sizes (multiplying a 4x4 by a 3x1, say) are compile errors. The size defaults to 4x4, the
    // transform matrices that most of the ray tracer uses.
    template<typename T, std::size_t NUM_ROWS = 4, std::size_t NUM_COLS = 4>
        requires std::is_arithmetic_v<T> && (NUM_ROWS > 0) && (NUM_COLS > 0)
    struct Matrix {
        static constexpr std::size_t num_rows{NUM_ROWS};
        static constexpr std::size_t num_cols{NUM_COLS};

        // All zeros
        constexpr Matrix() = default;

        // The elements row by row; the array's size makes a wrong count a compile error
        constexpr explicit Matrix(const std::array<T, NUM_ROWS * NUM_COLS> &data) : m_data{data} {
        }

        [[nodiscard]] static constexpr std::size_t rows() { return NUM_ROWS; }
        [[nodiscard]] static constexpr std::size_t cols() { return NUM_COLS; }
        [[nodiscard]] constexpr const std::array<T, NUM_ROWS * NUM_COLS> &data() const { return m_data; }

        constexpr T &operator[](const std::size_t row, const std::size_t col) { return m_data[row * NUM_COLS + col]; }

        constexpr const T &operator[](const std::size_t row, const std::size_t col) const {
            return m_data[row * NUM_COLS + col];
        }

        static constexpr Matrix identity() requires (NUM_ROWS == NUM_COLS) {
            Matrix result;
            for (std::size_t i = 0; i < NUM_ROWS; ++i) {
                result[i, i] = 1;
            }
            return result;
        }

        // Equal within utils::EPSILON element by element. Matrices of different sizes are different types, so they
        // can't be compared at all.
        friend constexpr bool operator==(const Matrix &mat1, const Matrix &mat2) {
            for (std::size_t i = 0; i < mat1.m_data.size(); ++i) {
                if (utils::abs(static_cast<double>(mat1.m_data[i]) - static_cast<double>(mat2.m_data[i])) >
                    utils::EPSILON) {
                    return false;
                }
            }
            return true;
        }

    private:
        std::array<T, NUM_ROWS * NUM_COLS> m_data{};
    };

    // A point (w = 1) or a vector (w = 0, so translations don't move it) as a 4x1 column, ready to multiply by a
    // transform. A braced list, make_matrix({x, y, z}), can't deduce T, so it falls back to the default: a point.
    template<typename T = Point>
        requires std::same_as<T, Point> || std::same_as<T, Vector>
    constexpr Matrix<double, 4, 1> make_matrix(const T &coords) {
        double w{0.0};
        if constexpr (std::same_as<T, Point>) {
            w = coords.w;
        }
        return Matrix<double, 4, 1>{
            {static_cast<double>(coords.x), static_cast<double>(coords.y), static_cast<double>(coords.z), w}
        };
    }

    template<typename T, std::size_t R, std::size_t N, std::size_t C>
    constexpr Matrix<T, R, C> multiply(const Matrix<T, R, N> &matrix1, const Matrix<T, N, C> &matrix2);

    template<typename T, std::size_t R, std::size_t C>
    constexpr Matrix<T, C, R> transpose(const Matrix<T, R, C> &matrix);

    template<typename T, std::size_t N>
        requires (N >= 2)
    constexpr T determinant(const Matrix<T, N, N> &matrix);

    template<typename T, std::size_t R, std::size_t C>
        requires (R >= 2 && C >= 2)
    constexpr Matrix<T, R - 1, C - 1> submatrix(const Matrix<T, R, C> &matrix, std::size_t row, std::size_t col);

    template<typename T, std::size_t N>
        requires (N >= 2)
    constexpr T minor(const Matrix<T, N, N> &matrix, std::size_t row, std::size_t col);

    template<typename T, std::size_t N>
        requires (N >= 2)
    constexpr T cofactor(const Matrix<T, N, N> &matrix, std::size_t row, std::size_t col);

    // The inverse, or an error if the matrix isn't invertible (its determinant is 0)
    template<typename T, std::size_t N>
        requires (N >= 2)
    constexpr std::expected<Matrix<double, N, N>, bool> inverse(const Matrix<T, N, N> &matrix);

    template<typename T>
    constexpr Matrix<T> translation(T x, T y, T z);

    template<typename T>
    constexpr Matrix<T> scale(T x, T y, T z);
}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_HPP
