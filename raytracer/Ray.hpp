#ifndef THE_RAYTRACER_CHALLENGE_RAY_HPP
#define THE_RAYTRACER_CHALLENGE_RAY_HPP

#include "Matrix.hpp"
#include "Point.hpp"
#include "Vector.hpp"

namespace raytracer {
    struct Ray {
        Point origin{};
        Vector direction{};
    };

    Point position(const Ray &ray, float distance);

    Ray transform(const Ray &ray, const Matrix<double> &matrix);
}

#endif //THE_RAYTRACER_CHALLENGE_RAY_HPP
