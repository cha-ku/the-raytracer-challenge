#include "Intersect.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <string>

using namespace raytracer;

SCENARIO("A ray intersects a cube") {
    const auto [face, origin, direction, t1, t2] = GENERATE(table<std::string, Point, Vector, float, float>({
        {"+x", Point(5, 0.5, 0), Vector(-1, 0, 0), 4.0f, 6.0f},
        {"-x", Point(-5, 0.5, 0), Vector(1, 0, 0), 4.0f, 6.0f},
        {"+y", Point(0.5, 5, 0), Vector(0, -1, 0), 4.0f, 6.0f},
        {"-y", Point(0.5, -5, 0), Vector(0, 1, 0), 4.0f, 6.0f},
        {"+z", Point(0.5, 0, 5), Vector(0, 0, -1), 4.0f, 6.0f},
        {"-z", Point(0.5, 0, -5), Vector(0, 0, 1), 4.0f, 6.0f},
        {"inside", Point(0, 0.5, 0), Vector(0, 0, 1), -1.0f, 1.0f},
    }));
    GIVEN("A cube and a ray hitting it from " + face) {
        const Cube c = Cube::make_cube();
        const Ray r{origin, direction};
        WHEN("The ray is intersected with the cube") {
            const auto xs = local_intersect(c, r);
            THEN("There are two intersections at t1 and t2") {
                REQUIRE(xs.size() == 2);
                REQUIRE(utils::equal(xs[0].t, t1));
                REQUIRE(utils::equal(xs[1].t, t2));
            }
        }
    }
}

SCENARIO("A ray misses a cube") {
    const auto [origin, direction] = GENERATE(table<Point, Vector>({
        {Point(-2, 0, 0), Vector(0.2673, 0.5345, 0.8018)},
        {Point(0, -2, 0), Vector(0.8018, 0.2673, 0.5345)},
        {Point(0, 0, -2), Vector(0.5345, 0.8018, 0.2673)},
        {Point(2, 0, 2), Vector(0, 0, -1)},
        {Point(0, 2, 2), Vector(0, -1, 0)},
        {Point(2, 2, 0), Vector(-1, 0, 0)},
    }));
    GIVEN("A cube and a ray that passes it by") {
        const Cube c = Cube::make_cube();
        const Ray r{origin, direction};
        WHEN("The ray is intersected with the cube") {
            const auto xs = local_intersect(c, r);
            THEN("There are no intersections") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("The normal on the surface of a cube") {
    const auto [point, expected_normal] = GENERATE(table<Point, Vector>({
        {Point(1, 0.5, -0.8), Vector(1, 0, 0)},
        {Point(-1, -0.2, 0.9), Vector(-1, 0, 0)},
        {Point(-0.4, 1, -0.1), Vector(0, 1, 0)},
        {Point(0.3, -1, -0.7), Vector(0, -1, 0)},
        {Point(-0.6, 0.3, 1), Vector(0, 0, 1)},
        {Point(0.4, 0.4, -1), Vector(0, 0, -1)},
        {Point(1, 1, 1), Vector(1, 0, 0)},
        {Point(-1, -1, -1), Vector(-1, 0, 0)},
    }));
    GIVEN("A cube and a point on its surface") {
        const Cube c = Cube::make_cube();
        WHEN("The normal is computed at the point") {
            const auto normal = local_normal_at(c, point);
            THEN("The normal points out of the face the point lies on") {
                REQUIRE(Vector::areAlmostEqual(normal, expected_normal));
            }
        }
    }
}
