//
// Created by chaku on 22/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATRIX_HPP
#define THE_RAYTRACER_CHALLENGE_MATRIX_HPP

#include <vector>
#include <mdspan>

namespace raytracer {
    template <typename T>
    requires std::is_arithmetic_v<T>
    struct Container {
        size_t m_rows;
        size_t m_cols;
        std::vector<T> m_data;
        constexpr explicit Container(const size_t rows, const size_t cols) : m_rows(rows), m_cols(cols), m_data(rows * cols, 0) {};

        constexpr explicit Container(const size_t rows, const size_t cols, auto&& custom_data) : m_rows(rows), m_cols(cols),
        m_data(std::begin(custom_data), std::end(custom_data))
        {
            if (std::ranges::size(custom_data) != rows * cols) {
                throw std::invalid_argument("Data size does not match matrix dimensions");
            }
        };

        constexpr auto data() -> decltype(m_data.data()) { return m_data.data(); };

        constexpr auto operator<=>(const Container&) const = default;

        static constexpr Container<T> identity(const size_t dim) {
            Container<T> result(dim, dim);
            for (size_t i = 0; i < dim; ++i) {
                result.m_data[i * dim + i] = 1;
            }
            return result;
        }
    };

    template <typename T>
    using Matrix = std::mdspan<T, std::dextents<size_t, 2>>;

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
        size_t inner = mat1.extent(1);
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
    Container<T> transpose(Container<T>& container) {
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
    T determinant(Container<T>& container) {
        if (container.m_rows != 2 || container.m_cols != 2) {
            throw std::invalid_argument("Determinant is only implemented for 2x2 matrices");
        }
        return container.data()[0] * container.data()[3] - container.data()[1] * container.data()[2];
    }

    template<typename T>
    Container<T> submatrix(Container<T>& container, const decltype(Container<T>::m_rows) row, const decltype(Container<T>::m_cols) col) {
        auto result = container;
        result.m_data.clear();
        result.m_rows = container.m_rows - 1;
        result.m_cols = container.m_cols - 1;
        auto matrix {make_matrix(container)};
        for (size_t r = 0; r < container.m_rows; ++r) {
            for (size_t c = 0; c < container.m_cols; ++c) {
                if (r != row && c != col) {
                    result.m_data.emplace_back(matrix[r, c]);
                }
            }
        }
        return result;
    }

}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_HPP
