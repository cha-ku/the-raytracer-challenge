//
// Created by chaku on 22/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP
#define THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP

#include <cmath>
#include "Matrix.hpp"

namespace raytracer {
    template<typename T, std::size_t R, std::size_t N, std::size_t C>
    constexpr Matrix<T, R, C> multiply(const Matrix<T, R, N> &matrix1, const Matrix<T, N, C> &matrix2) {
        Matrix<T, R, C> result;
        for (std::size_t i = 0; i < R; ++i) {
            for (std::size_t j = 0; j < C; ++j) {
                for (std::size_t k = 0; k < N; ++k) {
                    result[i, j] += matrix1[i, k] * matrix2[k, j];
                }
            }
        }
        return result;
    }

    template<typename T, std::size_t R, std::size_t C>
    constexpr Matrix<T, C, R> transpose(const Matrix<T, R, C> &matrix) {
        Matrix<T, C, R> result;
        for (std::size_t row = 0; row < R; ++row) {
            for (std::size_t col = 0; col < C; ++col) {
                result[col, row] = matrix[row, col];
            }
        }
        return result;
    }

    template<typename T, std::size_t N>
        requires (N >= 2)
    constexpr T determinant(const Matrix<T, N, N> &matrix) {
        if constexpr (N == 2) {
            return matrix[0, 0] * matrix[1, 1] - matrix[0, 1] * matrix[1, 0];
        } else {
            // Cofactor expansion along the first row; each cofactor is the determinant of an (N-1)x(N-1) matrix, so
            // the recursion ends at the 2x2 case above, chosen at compile time
            T result{};
            for (std::size_t col = 0; col < N; ++col) {
                result += matrix[0, col] * cofactor(matrix, 0, col);
            }
            return result;
        }
    }

    template<typename T, std::size_t R, std::size_t C>
        requires (R >= 2 && C >= 2)
    constexpr Matrix<T, R - 1, C - 1> submatrix(const Matrix<T, R, C> &matrix, const std::size_t row,
                                                const std::size_t col) {
        Matrix<T, R - 1, C - 1> result;
        std::size_t dest_row = 0;
        for (std::size_t r = 0; r < R; ++r) {
            if (r == row) continue;
            std::size_t dest_col = 0;
            for (std::size_t c = 0; c < C; ++c) {
                if (c == col) continue;
                result[dest_row, dest_col] = matrix[r, c];
                ++dest_col;
            }
            ++dest_row;
        }
        return result;
    }

    template<typename T, std::size_t N>
        requires (N >= 2)
    constexpr T minor(const Matrix<T, N, N> &matrix, const std::size_t row, const std::size_t col) {
        return determinant(submatrix(matrix, row, col));
    }

    template<typename T, std::size_t N>
        requires (N >= 2)
    constexpr T cofactor(const Matrix<T, N, N> &matrix, const std::size_t row, const std::size_t col) {
        const T result = minor(matrix, row, col);
        return (row + col) % 2 == 0 ? result : -result;
    }

    template<typename T, std::size_t N>
        requires (N >= 2)
    constexpr std::expected<Matrix<double, N, N>, bool> inverse(const Matrix<T, N, N> &matrix) {
        const auto matrix_determinant = static_cast<double>(determinant(matrix));
        if (matrix_determinant == 0) {
            return std::unexpected(false);
        }
        // Each element is its cofactor divided by the determinant, transposed: the cofactor of (row, col) goes to
        // (col, row)
        Matrix<double, N, N> result;
        for (std::size_t row = 0; row < N; ++row) {
            for (std::size_t col = 0; col < N; ++col) {
                result[col, row] = static_cast<double>(cofactor(matrix, row, col)) / matrix_determinant;
            }
        }
        return result;
    }

    template<typename T>
    constexpr Matrix<T> translation(T x, T y, T z) {
        Matrix result{Matrix<T>::identity()};
        result[0, 3] = x;
        result[1, 3] = y;
        result[2, 3] = z;
        return result;
    }

    template<typename T>
    constexpr Matrix<T> scale(T x, T y, T z) {
        Matrix result{Matrix<T>::identity()};
        result[0, 0] = x;
        result[1, 1] = y;
        result[2, 2] = z;
        return result;
    }

    constexpr Matrix<double> rotation_x(const double radians) {
        Matrix result{Matrix<double>::identity()};
        result[1, 1] = std::cos(radians);
        result[1, 2] = -std::sin(radians);
        result[2, 1] = std::sin(radians);
        result[2, 2] = std::cos(radians);
        return result;
    }

    constexpr Matrix<double> rotation_y(const double radians) {
        Matrix result{Matrix<double>::identity()};
        result[0, 0] = std::cos(radians);
        result[0, 2] = std::sin(radians);
        result[2, 0] = -std::sin(radians);
        result[2, 2] = std::cos(radians);
        return result;
    }

    constexpr Matrix<double> rotation_z(const double radians) {
        Matrix result{Matrix<double>::identity()};
        result[0, 0] = std::cos(radians);
        result[0, 1] = -std::sin(radians);
        result[1, 0] = std::sin(radians);
        result[1, 1] = std::cos(radians);
        return result;
    }

    constexpr Point transform(const Point &point, const Matrix<double> &matrix) {
        const Matrix point_matrix{multiply(matrix, make_matrix(point))};
        return Point{
            .x = static_cast<float>(point_matrix[0, 0]),
            .y = static_cast<float>(point_matrix[1, 0]),
            .z = static_cast<float>(point_matrix[2, 0])
        };
    }

    constexpr Matrix<double> shearing(const double xy, const double xz, const double yx, const double yz,
                                      const double zx, const double zy) {
        Matrix result{Matrix<double>::identity()};
        result[0, 1] = xy;
        result[0, 2] = xz;
        result[1, 0] = yx;
        result[1, 2] = yz;
        result[2, 0] = zx;
        result[2, 1] = zy;
        return result;
    }
}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP
