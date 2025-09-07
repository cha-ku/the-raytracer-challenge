//
// Created by chaku on 07/09/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATRIX_HPP
#define THE_RAYTRACER_CHALLENGE_MATRIX_HPP

#include <expected>
#include <vector>
#include <mdspan>

#include "Utils.hpp"

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

        constexpr auto data() -> decltype(m_data.data()) { return m_data.data(); }

        static constexpr Container identity(const size_t dim) {
            Container result(dim, dim);
            for (size_t i = 0; i < dim; ++i) {
                result.m_data[i * dim + i] = 1;
            }
            return result;
        }
    };

    template<typename T=double>
    constexpr auto operator==(const Container<T>& a, const Container<T>& other) -> bool {
        if (a.m_rows != other.m_rows || a.m_cols != other.m_cols) {
            return false;
        }
        return utils::is_almost_equal(a.m_data, other.m_data);
    }

    template <typename T>
    using Matrix = std::mdspan<T, std::dextents<size_t, 2>>;

    template <typename T>
    Matrix<std::remove_cvref_t<T>> make_matrix(Container<T>& d);

    template <typename T>
    Container<T> multiply(Container<T> container1, Container<T> container2);

    template <typename T>
    Container<T> transpose(Container<T> container);

    template <typename T>
    T determinant(Container<T> container);

    template<typename T>
    Container<T> submatrix(Container<T> container, decltype(Container<T>::m_rows) row, decltype(Container<T>::m_cols) col);

    template<typename T>
    T minor(Container<T> container, decltype(Container<T>::m_rows) row, decltype(Container<T>::m_cols) col);

    template<typename T>
    T cofactor(Container<T> container, decltype(Container<T>::m_rows) row, decltype(Container<T>::m_cols) col);

    template<typename T>
    std::expected<Container<double>, bool> inverse(Container<T> container);
}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_HPP
