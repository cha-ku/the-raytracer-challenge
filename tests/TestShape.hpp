#ifndef THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP
#define THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP

#include "Shape.hpp"

namespace raytracer {
    // A minimal concrete Shape used only to exercise Shape's own behavior
    // (transform/material) in tests, without needing a real renderable
    // shape like Sphere or Plane. Kept out of Shape.hpp so production code
    // can never construct or depend on it.
    struct TestShape : Shape, ShapeFactory<TestShape> {
        explicit TestShape(const uint32_t id) : Shape(id, ShapeType::TestShape) {}
    };

    inline TestShape test_shape() { return TestShape::make(); }
}

#endif //THE_RAYTRACER_CHALLENGE_TEST_SHAPE_HPP
