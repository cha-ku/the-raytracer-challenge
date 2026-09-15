//
// Created by chaku on 22/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP
#define THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP

#include <format>
#include <numeric>
#include "Matrix.hpp"

namespace raytracer {
    template<typename T>
    Matrix<T> multiply(Matrix<T> matrix1, Matrix<T> matrix2) {
        if (matrix1.m_cols != matrix2.m_rows) {
            throw std::invalid_argument(std::format(
                "Matrix dimensions do not allow multiplication: mat1 columns ({}) != mat2 rows ({})",
                matrix1.m_cols, matrix2.m_rows
            ));
        }
        const size_t inner = matrix1.m_cols;
        size_t rows = matrix1.m_rows;
        size_t cols = matrix2.m_cols;

        Matrix<T> result{rows, cols};
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                for (size_t k = 0; k < inner; ++k) {
                    result[i, j] += matrix1[i, k] * matrix2[k, j];
                }
            }
        }
        return result;
    }

    template<typename T>
    Matrix<T> transpose(Matrix<T> matrix) {
        Matrix<T> result(matrix.m_cols, matrix.m_rows);
        for (size_t row = 0; row < matrix.m_rows; ++row) {
            for (size_t col = 0; col < matrix.m_cols; ++col) {
                result[col, row] = matrix[row, col];
            }
        }
        return result;
    }

    template<typename T>
    T determinant(Matrix<T> matrix) {
        if (matrix.m_rows != matrix.m_cols || matrix.m_rows < 2 || matrix.m_cols < 2) {
            throw std::invalid_argument("Determinant is only implemented for square matrices of size 2 or more");
        }
        if (matrix.m_rows == 2 && matrix.m_cols == 2) {
            return matrix[0, 0] * matrix[1, 1] - matrix[0, 1] * matrix[1, 0];
        }
        if (matrix.m_rows == 3 && matrix.m_cols == 3) {
            return matrix[0, 0] * cofactor(matrix, 0, 0) + matrix[0, 1] * cofactor(matrix, 0, 1) +
                   matrix[0, 2] * cofactor(matrix, 0, 2);
        }
        if (matrix.m_rows == 4 && matrix.m_cols == 4) {
            return matrix[0, 0] * cofactor(matrix, 0, 0) +
                   matrix[0, 1] * cofactor(matrix, 0, 1) +
                   matrix[0, 2] * cofactor(matrix, 0, 2) +
                   matrix[0, 3] * cofactor(matrix, 0, 3);
        }
        return {};
    }

    template<typename T>
    Matrix<T> submatrix(Matrix<T> matrix, decltype(Matrix<T>::m_rows) row,
                           decltype(Matrix<T>::m_cols) col) {
        auto result = matrix;
        result.m_data.clear();
        result.m_rows = matrix.m_rows - 1;
        result.m_cols = matrix.m_cols - 1;
        for (size_t r = 0; r < matrix.m_rows; ++r) {
            for (size_t c = 0; c < matrix.m_cols; ++c) {
                if (r != row && c != col) {
                    result.m_data.emplace_back(matrix[r, c]);
                }
            }
        }
        return result;
    }

    template<typename T>
    T minor(Matrix<T> matrix, decltype(Matrix<T>::m_rows) row, decltype(Matrix<T>::m_cols) col) {
        return determinant(submatrix(matrix, row, col));
    }

    template<typename T>
    T cofactor(Matrix<T> matrix, decltype(Matrix<T>::m_rows) row, decltype(Matrix<T>::m_cols) col) {
        T result = minor(matrix, row, col);
        return (row + col) % 2 == 0 ? result : -result;
    }

    template<typename T>
    std::expected<Matrix<double>, bool> inverse(Matrix<T> matrix) {
        auto matrix_determinant = determinant(matrix);
        if (matrix_determinant == 0) {
            return std::unexpected(false);
        }
        const size_t num_rows = matrix.m_rows;
        const size_t num_cols = matrix.m_rows;
        auto cofactors = Matrix<double>(num_rows, num_cols, std::vector<T>(num_rows * num_cols, 0));
        for (size_t row = 0; row < num_rows; ++row) {
            for (size_t col = 0; col < num_cols; ++col) {
                cofactors[row, col] = cofactor(matrix, row, col);
            }
        }
        auto result{transpose(cofactors)};
        for (size_t row = 0; row < num_rows; ++row) {
            for (size_t col = 0; col < num_cols; ++col) {
                result[row, col] /= matrix_determinant;
            }
        }
        return result;
    }

    template <typename T>
    constexpr Matrix<T> translation(T x, T y, T z) {
        Matrix<T> result(Matrix<T>::identity(4));
        result[0, 3] = x;
        result[1, 3] = y;
        result[2, 3] = z;
        return result;
    }

    template <typename T>
    constexpr Matrix<T> scale(T x, T y, T z) {
        Matrix<T> result(Matrix<T>::identity(4));
        result[0, 0] = x;
        result[1, 1] = y;
        result[2, 2] = z;
        return result;
    }

    constexpr Matrix<double> rotation_x(const double radians) {
        Matrix result(Matrix<double>::identity(4));
        result[1, 1] = std::cos(radians);
        result[1, 2] = -std::sin(radians);
        result[2, 1] = std::sin(radians);
        result[2, 2] = std::cos(radians);
        return result;
    }

    constexpr Matrix<double> rotation_y(const double radians) {
        Matrix result(Matrix<double>::identity(4));
        result[0, 0] = std::cos(radians);
        result[0, 2] = std::sin(radians);
        result[2, 0] = -std::sin(radians);
        result[2, 2] = std::cos(radians);
        return result;
    }

    constexpr Matrix<double> rotation_z(const double radians) {
        Matrix result(Matrix<double>::identity(4));
        result[0, 0] = std::cos(radians);
        result[0, 1] = -std::sin(radians);
        result[1, 0] = std::sin(radians);
        result[1, 1] = std::cos(radians);
        return result;
    }

    constexpr Matrix<double> shearing(const double xy, const double xz, const double yx, const double yz, const double zx, const double zy) {
        Matrix result(Matrix<double>::identity(4));
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
