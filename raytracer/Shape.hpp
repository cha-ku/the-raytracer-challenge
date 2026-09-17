//
// Created by chaku on 15/09/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_SHAPE_HPP
#define THE_RAYTRACER_CHALLENGE_SHAPE_HPP

#include "Material.hpp"
#include "Matrix.hpp"

namespace raytracer {
    static uint32_t shape_id{0};

    struct Shape {
        uint32_t m_id;
        Matrix<double> m_transform{Matrix<double>::identity(4)};
        Material m_material{};

        Shape() = delete;

        explicit Shape(const uint32_t id) : m_id(id) {};

        void set_transform(const Matrix<double> &t) {
            m_transform = t;
        }

        bool operator==(const Shape& other) const { return m_id == other.m_id; }
    };

    // CRTP mixin: gives any `struct X : Shape, ShapeFactory<X> {}` a
    // `make()` that assigns the next id and constructs a Derived.
    // Derived must expose a constructor taking a single uint32_t id.
    template <typename Derived>
    struct ShapeFactory {
        static Derived make() {
            return Derived(++shape_id);
        }
    };

}

#endif //THE_RAYTRACER_CHALLENGE_SHAPE_HPP