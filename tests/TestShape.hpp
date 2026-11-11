#ifndef THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP
#define THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP

#include "Geom.hpp"

namespace raytracer {
    // TestShape itself (and its local_intersect/local_normal_at) is in production code, like TestPattern, so it
    // can be part of AnyShape. This helper keeps the book's test_shape() spelling.
    inline Geom make_test_shape() { return Geom{TestShape{}}; }

    // The local-space ray the last intersect() call passed to this test shape
    inline const Ray &saved_ray(const Geom &shape) { return std::get<TestShape>(shape.shape).saved_ray; }
}

#endif //THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP
