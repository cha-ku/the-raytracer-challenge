#include "Geom.hpp"

#include <stdexcept>

namespace raytracer {
    bool BoundingBox::contains(const Point &point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    bool BoundingBox::contains(const BoundingBox &box) const {
        return contains(box.min) && contains(box.max);
    }

    std::pair<BoundingBox, BoundingBox> split_bounds(const BoundingBox &box) {
        const auto dx {box.max.x - box.min.x};
        const auto dy {box.max.y - box.min.y};
        const auto dz {box.max.z - box.min.z};
        const auto greatest = std::max({dx, dy, dz});
        Point mid_min{box.min};
        Point mid_max{box.max};

        if (greatest == dx) {
            mid_min.x = mid_max.x = box.min.x + dx / 2;
        } else if (greatest == dy) {
            mid_min.y = mid_max.y = box.min.y + dy / 2;
        } else {
            mid_min.z = mid_max.z = box.min.z + dz / 2;
        }

        return {BoundingBox{.min = box.min, .max = mid_max},
                BoundingBox{.min = mid_min, .max = box.max}};
    }

    void add_child(Geom &group, Geom &child) {
        child.parent = &group;
        std::get<Group>(group.shape).children.push_back(&child);
    }

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
        const Matrix normal_matrix{
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
