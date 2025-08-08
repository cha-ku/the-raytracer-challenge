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
        std::vector<T> m_data;
        size_t m_rows;
        size_t m_cols;
        constexpr explicit Container(const size_t rows, const size_t cols) : m_rows(rows), m_cols(cols), m_data(rows * cols, 0) {};

        constexpr Container(const size_t rows, const size_t cols, auto&& custom_data) : m_rows(rows), m_cols(cols),
        m_data(std::begin(custom_data), std::end(custom_data))
        {
            if (std::ranges::size(custom_data) != rows * cols) {
                throw std::invalid_argument("RuntimeError: Data size does not match matrix dimensions");
            }
        };

        auto data() -> decltype(m_data.data()) { return m_data.data(); };
        auto operator<=>(const Container&) const = default;
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
        return {rows, cols, result_vec};
    }
}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_HPP
