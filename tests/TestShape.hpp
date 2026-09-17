#ifndef THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP
#define THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP

#include "Intersect.hpp"
#include "Shape.hpp"

namespace raytracer {
    // A minimal concrete Shape used only to exercise Shape's own behavior
    // (transform/material) in tests, without needing a real renderable
    // shape like Sphere or Plane. Kept out of Shape.hpp so production code
    // can never construct or depend on it.
    struct TestShape : Shape, ShapeFactory<TestShape> {
        explicit TestShape(const uint32_t id) : Shape(id) {}

        // Records the local-space ray passed to local_intersect, so tests
        // can verify intersect() converted the world-space ray into this
        // shape's object space correctly. mutable because recording it is
        // an observation, not a change to the shape's own state, and
        // local_intersect receives the shape by const reference.
        mutable Ray saved_ray{};
    };

    inline TestShape make_test_shape() { return TestShape::make(); }

    inline std::vector<Intersection> local_intersect(const TestShape &shape, const Ray &local_ray) {
        shape.saved_ray = local_ray;
        return {};
    }

    inline Vector local_normal_at(const TestShape &shape, const Point &local_point) {
        return Vector(local_point.x, local_point.y, local_point.z);
    }
}

#endif //THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP
