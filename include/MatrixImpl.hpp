//
// Created by chaku on 22/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP
#define THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP

#include "Matrix.hpp"

namespace raytracer {
    template <typename T>
    Matrix<std::remove_cvref_t<T>> make_matrix(Container<T>& d) {
        using U = std::remove_cvref_t<T>;
        return Matrix<U>(d.data(), d.m_rows, d.m_cols);
    }

    template <typename T>
    Container<T> multiply(Container<T> container1, Container<T> container2) {
        Matrix mat1 = make_matrix(container1);
        Matrix mat2 = make_matrix(container2);
        if (mat1.extent(1) != mat2.extent(0)) {
            throw std::invalid_argument("Matrix dimensions do not allow multiplication");
        }
        const size_t inner = mat1.extent(1);
        size_t rows = mat1.extent(0);
        size_t cols = mat2.extent(1);

        std::vector<T> result_vec(rows * cols, 0);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                for (size_t k = 0; k < inner; ++k) {
                    result_vec[i * cols + j] += mat1[i, k] * mat2[k, j];
                }
            }
        }
        return Container<T>(rows, cols, result_vec);
    }

    template <typename T>
    Container<T> transpose(Container<T> container) {
        auto result = container;
        for (size_t row = 0; row < container.m_rows; ++row) {
            for (size_t col = 0; col < container.m_cols; ++col) {
                // result[col, row] = container[row, col]
                std::swap(container.m_data[col * container.m_cols + row] , result.m_data[row * container.m_cols + col]);
            }
        }
        return result;
    }

    template <typename T>
    T determinant(Container<T> container) {
        if (container.m_rows != container.m_cols || container.m_rows < 2 || container.m_cols < 2) {
            throw std::invalid_argument("Determinant is only implemented for square matrices of size 2 or more");
        }
        if (container.m_rows == 2 && container.m_cols == 2) {
            return container.m_data[0] * container.m_data[3] - container.m_data[1] * container.m_data[2];
        }
        if (container.m_rows == 3 && container.m_cols == 3) {
            return container.m_data[0] * cofactor(container, 0, 0) + container.m_data[1] * cofactor(container, 0, 1) + container.m_data[2] * cofactor(container, 0, 2);
        }
        return {};
    }

    template<typename T>
    Container<T> submatrix(Container<T> container, decltype(Container<T>::m_rows) row, decltype(Container<T>::m_cols) col) {
        const auto& matrix {make_matrix(container)};
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
}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_IMPL_HPP
