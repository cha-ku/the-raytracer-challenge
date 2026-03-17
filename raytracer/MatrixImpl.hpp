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
    Matrix<std::remove_cvref_t<T> > make_matrix(Container<T> &d) {
        using U = std::remove_cvref_t<T>; return Matrix<U>(d.data(), d.m_rows, d.m_cols);
    }

    template<typename T>
    Container<T> multiply(Container<T> container1, Container<T> container2) {
        Matrix mat1 = make_matrix(container1);
        Matrix mat2 = make_matrix(container2);
        if (mat1.extent(1) != mat2.extent(0)) {
            throw std::invalid_argument(std::format(
                "Matrix dimensions do not allow multiplication: mat1 columns ({}) != mat2 rows ({})",
                mat1.extent(1), mat2.extent(0)
            ));
        }
        const size_t inner = mat1.extent(1);
        size_t rows = mat1.extent(0);
        size_t cols = mat2.extent(1);

        Container<T> result{rows, cols};
        Matrix result_matrix{make_matrix(result)};
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                for (size_t k = 0; k < inner; ++k) {
                    result_matrix[i, j] += mat1[i, k] * mat2[k, j];
                }
            }
        }
        return result;
    }

    template<typename T>
    Container<T> transpose(Container<T> container) {
        auto result = container;
        for (size_t row = 0; row < container.m_rows; ++row) {
            for (size_t col = 0; col < container.m_cols; ++col) {
                // result[col, row] = container[row, col]
                std::swap(container.m_data[col * container.m_cols + row], result.m_data[row * container.m_cols + col]);
            }
        }
        return result;
    }

    template<typename T>
    T determinant(Container<T> container) {
        if (container.m_rows != container.m_cols || container.m_rows < 2 || container.m_cols < 2) {
            throw std::invalid_argument("Determinant is only implemented for square matrices of size 2 or more");
        }
        if (container.m_rows == 2 && container.m_cols == 2) {
            return container.m_data[0] * container.m_data[3] - container.m_data[1] * container.m_data[2];
        }
        if (container.m_rows == 3 && container.m_cols == 3) {
            return container.m_data[0] * cofactor(container, 0, 0) + container.m_data[1] * cofactor(container, 0, 1) +
                   container.m_data[2] * cofactor(container, 0, 2);
        }
        if (container.m_rows == 4 && container.m_cols == 4) {
            return container.m_data[0] * cofactor(container, 0, 0) +
                   container.m_data[1] * cofactor(container, 0, 1) +
                   container.m_data[2] * cofactor(container, 0, 2) +
                   container.m_data[3] * cofactor(container, 0, 3);
        }
        return {};
    }

    template<typename T>
    Container<T> submatrix(Container<T> container, decltype(Container<T>::m_rows) row,
                           decltype(Container<T>::m_cols) col) {
        const auto &matrix{make_matrix(container)};
        auto result = container;
        result.m_data.clear();
        result.m_rows = container.m_rows - 1;
        result.m_cols = container.m_cols - 1;
        for (size_t r = 0; r < container.m_rows; ++r) {
            for (size_t c = 0; c < container.m_cols; ++c) {
                if (r != row && c != col) {
                    result.m_data.emplace_back(matrix[r, c]);
                }
            }
        }
        return result;
    }

    template<typename T>
    T minor(Container<T> container, decltype(Container<T>::m_rows) row, decltype(Container<T>::m_cols) col) {
        return determinant(submatrix(container, row, col));
    }

    template<typename T>
    T cofactor(Container<T> container, decltype(Container<T>::m_rows) row, decltype(Container<T>::m_cols) col) {
        T result = minor(container, row, col);
        return (row + col) % 2 == 0 ? result : -result;
    }

    template<typename T>
    std::expected<Container<double>, bool> inverse(Container<T> container) {
        auto container_determinant = determinant(container);
        if (container_determinant == 0) {
            return std::unexpected(false);
        }
        const size_t num_rows = container.m_rows;
        const size_t num_cols = container.m_rows;
        auto matrix = make_matrix(container);
        auto cofactors = Container<double>(num_rows, num_cols, std::vector<T>(num_rows * num_cols, 0));
        auto cofactor_matrix{make_matrix(cofactors)};
        for (auto row = 0; row < num_rows; ++row) {
            for (auto col = 0; col < num_cols; ++col) {
                cofactor_matrix[row, col] = cofactor(container, row, col);
            }
        }
        auto result{transpose(cofactors)};
        auto result_matrix{make_matrix(result)};
        for (auto row = 0; row < num_rows; ++row) {
            for (auto col = 0; col < num_cols; ++col) {
                result_matrix[row, col] /= container_determinant;
            }
        }
        return result;
    }

    template <typename T>
    constexpr Container<T> translation(T x, T y, T z) {
        Container<T> result(Container<T>::identity(4));
        auto mat{make_matrix(result)};
        mat[0, 3] = x;
        mat[1, 3] = y;
        mat[2, 3] = z;
        return result;
    }

    template <typename T>
    constexpr Container<T> scale(T x, T y, T z) {
        Container<T> result(Container<T>::identity(4));
        auto mat{make_matrix(result)};
        mat[0, 0] = x;
        mat[1, 1] = y;
        mat[2, 2] = z;
        return result;
    }

    constexpr Container<double> rotation_x(const double radians) {
        Container result(Container<double>::identity(4));
        auto mat{make_matrix(result)};
        mat[1, 1] = std::cos(radians);
        mat[1, 2] = -std::sin(radians);
        mat[2, 1] = std::sin(radians);
        mat[2, 2] = std::cos(radians);
        return result;
    }

    constexpr Container<double> rotation_y(const double radians) {
        Container result(Container<double>::identity(4));
        auto mat{make_matrix(result)};
        mat[0, 0] = std::cos(radians);
        mat[0, 2] = std::sin(radians);
        mat[2, 0] = -std::sin(radians);
        mat[2, 2] = std::cos(radians);
        return result;
    }

    constexpr Container<double> rotation_z(const double radians) {
        Container result(Container<double>::identity(4));
        auto mat{make_matrix(result)};
        mat[0, 0] = std::cos(radians);
        mat[0, 1] = -std::sin(radians);
        mat[1, 0] = std::sin(radians);
        mat[1, 1] = std::cos(radians);
        return result;
    }

    constexpr Container<double> shearing(const double xy, const double xz, const double yx, const double yz, const double zx, const double zy) {
        Container result(Container<double>::identity(4));
        auto mat{make_matrix(result)};
        mat[0, 1] = xy;
        mat[0, 2] = xz;
        mat[1, 0] = yx;
        mat[1, 2] = yz;
        mat[2, 0] = zx;
        mat[2, 1] = zy;
        return result;
    }
}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP
