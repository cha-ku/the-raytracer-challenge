//
// Created by chaku on 07/09/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATRIX_HPP
#define THE_RAYTRACER_CHALLENGE_MATRIX_HPP

#include <expected>
#include <vector>

#include "Point.hpp"
#include "Utils.hpp"

namespace raytracer {
    template<typename T>
        requires std::is_arithmetic_v<T>
    struct Matrix {
        constexpr explicit Matrix(const size_t rows, const size_t cols) : m_rows(rows), m_cols(cols),
                                                                             m_data(rows * cols, 0) {
        };

        constexpr explicit Matrix(const size_t rows, const size_t cols, auto &&custom_data) : m_rows(rows),
            m_cols(cols),
            m_data(std::begin(custom_data), std::end(custom_data)) {
            if (std::ranges::size(custom_data) != rows * cols) {
                throw std::invalid_argument("Data size does not match matrix dimensions");
            }
        };

        constexpr size_t rows() const { return m_rows; }
        constexpr size_t cols() const { return m_cols; }
        constexpr const std::vector<T> &data() const { return m_data; }

        constexpr T &operator[](const size_t row, const size_t col) { return m_data[row * m_cols + col]; }
        constexpr const T &operator[](const size_t row, const size_t col) const { return m_data[row * m_cols + col]; }

        static constexpr Matrix identity(const size_t dim) {
            Matrix result(dim, dim);
            for (size_t i = 0; i < dim; ++i) {
                result[i, i] = 1;
            }
            return result;
        }

        friend constexpr bool operator==(const Matrix &mat1, const Matrix &mat2) {
            if (mat1.m_rows != mat2.m_rows || mat1.m_cols != mat2.m_cols) {
                return false;
            }
            return utils::is_almost_equal(mat1.m_data, mat2.m_data);
        }

    private:
        size_t m_rows;
        size_t m_cols;
        std::vector<T> m_data;
    };

    template<typename T=Point>
    constexpr Matrix<double> make_matrix(T&& p) {
        return Matrix<double>{4, 1, std::vector{p.x, p.y, p.z, p.w}};
    }

    constexpr Matrix<double> make_matrix(const Vector& v) {
        return Matrix<double>{4, 1, std::vector{static_cast<double>(v.x), static_cast<double>(v.y), static_cast<double>(v.z), 0.0}};
    }

    template<typename T>
    Matrix<T> multiply(Matrix<T> matrix1, Matrix<T> matrix2);

    template<typename T>
    Matrix<T> transpose(Matrix<T> matrix);

    template<typename T>
    T determinant(Matrix<T> matrix);

    template<typename T>
    Matrix<T> submatrix(Matrix<T> matrix, size_t row, size_t col);

    template<typename T>
    T minor(Matrix<T> matrix, size_t row, size_t col);

    template<typename T>
    T cofactor(Matrix<T> matrix, size_t row, size_t col);

    template<typename T>
    std::expected<Matrix<double>, bool> inverse(Matrix<T> matrix);

    template<typename T>
    constexpr Matrix<T> translation(T x, T y, T z);

    template <typename T>
    constexpr Matrix<T> scale(T x, T y, T z);
}

#endif //THE_RAYTRACER_CHALLENGE_MATRIX_HPP
