#include "Intersect.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <numbers>

using namespace raytracer;

SCENARIO("Intersecting a cone with a ray") {
    const auto [origin, direction, t0, t1] = GENERATE(table<Point, Vector, float, float>({
        {Point(0, 0, -5), Vector(0, 0, 1), 5.0f, 5.0f},
        {Point(0, 0, -5), Vector(1, 1, 1), 8.66025f, 8.66025f},
        {Point(1, 1, -5), Vector(-0.5, -1, 1), 4.55006f, 49.44994f},
    }));
    GIVEN("A cone and a ray that strikes it") {
        const Cone shape = Cone::make_cone();
        const Ray r{origin, Vector::normalize(direction)};
        WHEN("The ray is intersected with the cone") {
            const auto xs = local_intersect(shape, r);
            THEN("There are two intersections at t0 and t1") {
                REQUIRE(xs.size() == 2);
                REQUIRE(utils::equal(xs[0].t, t0));
                REQUIRE(utils::equal(xs[1].t, t1));
            }
        }
    }
}

SCENARIO("Intersecting a cone with a ray parallel to one of its halves") {
    GIVEN("A cone and a ray parallel to one of its halves") {
        const Cone shape = Cone::make_cone();
        const Ray r{Point(0, 0, -1), Vector::normalize(Vector(0, 1, 1))};
        WHEN("The ray is intersected with the cone") {
            const auto xs = local_intersect(shape, r);
            THEN("There is a single intersection with the other half") {
                REQUIRE(xs.size() == 1);
                REQUIRE(utils::equal(xs[0].t, 0.35355f));
            }
        }
    }
}

SCENARIO("Intersecting a cone's end caps") {
    const auto [origin, direction, count] = GENERATE(table<Point, Vector, std::size_t>({
        {Point(0, 0, -5), Vector(0, 1, 0), 0},
        {Point(0, 0, -0.25), Vector(0, 1, 1), 2},
        {Point(0, 0, -0.25), Vector(0, 1, 0), 4},
    }));
    GIVEN("A closed cone constrained to -0.5 < y < 0.5 and a ray") {
        Cone shape = Cone::make_cone();
        shape.minimum = -0.5;
        shape.maximum = 0.5;
        shape.closed = true;
        const Ray r{origin, Vector::normalize(direction)};
        WHEN("The ray is intersected with the cone") {
            const auto xs = local_intersect(shape, r);
            THEN("The caps are counted as intersections") {
                REQUIRE(xs.size() == count);
            }
        }
    }
}

SCENARIO("Computing the normal vector on a cone") {
    using std::numbers::sqrt2_v;
    const auto [point, expected_normal] = GENERATE(table<Point, Vector>({
        {Point(0, 0, 0), Vector(0, 0, 0)},
        {Point(1, 1, 1), Vector(1, -sqrt2_v<float>, 1)},
        {Point(-1, -1, 0), Vector(-1, 1, 0)},
    }));
    GIVEN("A cone") {
        const Cone shape = Cone::make_cone();
        WHEN("The normal is computed at the point") {
            const auto n = local_normal_at(shape, point);
            THEN("The normal is perpendicular to the cone's sloping side") {
                REQUIRE(Vector::areAlmostEqual(n, expected_normal));
            }
        }
    }
}
