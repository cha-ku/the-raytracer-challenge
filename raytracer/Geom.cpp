#include "Geom.hpp"

#include <stdexcept>

namespace raytracer {
    void Geom::set_transform(const Matrix<double> &t) {
        const auto t_inverse{inverse(t)};
        if (!t_inverse.has_value()) {
            throw std::invalid_argument("Geom transform must be invertible");
        }
        m_transform = t;
        m_inverse = t_inverse.value();
    }

    Point world_to_object(const Geom &geom, const Point &world_point) {
        const Point point{geom.parent != nullptr ? world_to_object(*geom.parent, world_point) : world_point};
        return transform(point, geom.inverse_transform());
    }

    Vector normal_to_world(const Geom &geom, const Vector &object_normal) {
        const Matrix<double> normal_matrix{
            multiply(transpose(geom.inverse_transform()), make_matrix(object_normal))
        };
        const Vector normal{
            Vector::normalize(Vector{
                .x = static_cast<float>(normal_matrix[0, 0]),
                .y = static_cast<float>(normal_matrix[1, 0]),
                .z = static_cast<float>(normal_matrix[2, 0])
            })
        };
        return geom.parent != nullptr ? normal_to_world(*geom.parent, normal) : normal;
    }
}
