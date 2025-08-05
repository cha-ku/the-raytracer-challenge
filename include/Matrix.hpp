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

        constexpr explicit Container(const size_t rows, const size_t cols, auto&& custom_data) : m_rows(rows), m_cols(cols),
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
}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_HPP
