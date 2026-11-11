#include "Intersect.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <limits>
#include <string>

using namespace raytracer;

SCENARIO("A ray misses a cylinder") {
    const auto [origin, direction] = GENERATE(table<Point, Vector>({
        {Point(1, 0, 0), Vector(0, 1, 0)},
        {Point(0, 0, 0), Vector(0, 1, 0)},
        {Point(0, 0, -5), Vector(1, 1, 1)},
    }));
    GIVEN("A cylinder and a ray that passes it by") {
        const Cylinder cyl{};
        const Ray r{origin, Vector::normalize(direction)};
        WHEN("The ray is intersected with the cylinder") {
            const auto xs = local_intersect(cyl, r);
            THEN("There are no intersections") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("A ray strikes a cylinder") {
    const auto [origin, direction, t0, t1] = GENERATE(table<Point, Vector, float, float>({
        {Point(1, 0, -5), Vector(0, 0, 1), 5.0f, 5.0f},
        {Point(0, 0, -5), Vector(0, 0, 1), 4.0f, 6.0f},
        {Point(0.5, 0, -5), Vector(0.1, 1, 1), 6.80798f, 7.08872f},
    }));
    GIVEN("A cylinder and a ray that strikes it") {
        const Cylinder cyl{};
        const Ray r{origin, Vector::normalize(direction)};
        WHEN("The ray is intersected with the cylinder") {
            const auto xs = local_intersect(cyl, r);
            THEN("There are two intersections at t0 and t1") {
                REQUIRE(xs.size() == 2);
                REQUIRE(utils::equal(xs[0], t0));
                REQUIRE(utils::equal(xs[1], t1));
            }
        }
    }
}

SCENARIO("Normal vector on a cylinder") {
    const auto [point, expected_normal] = GENERATE(table<Point, Vector>({
        {Point(1, 0, 0), Vector(1, 0, 0)},
        {Point(0, 5, -1), Vector(0, 0, -1)},
        {Point(0, -2, 1), Vector(0, 0, 1)},
        {Point(-1, 1, 0), Vector(-1, 0, 0)},
    }));
    GIVEN("A cylinder and a point on its surface") {
        const Cylinder cyl{};
        WHEN("The normal is computed at the point") {
            const auto n = local_normal_at(cyl, point);
            THEN("The normal points straight out from the y axis") {
                REQUIRE(Vector::areAlmostEqual(n, expected_normal));
            }
        }
    }
}

SCENARIO("The default minimum and maximum for a cylinder") {
    GIVEN("A default cylinder") {
        const Cylinder cyl{};
        THEN("It is unbounded in both directions along y") {
            REQUIRE(cyl.minimum == -std::numeric_limits<float>::infinity());
            REQUIRE(cyl.maximum == std::numeric_limits<float>::infinity());
        }
    }
}

SCENARIO("Intersecting a constrained cylinder") {
    const auto [row, point, direction, count] = GENERATE(table<int, Point, Vector, std::size_t>({
        {1, Point(0, 1.5, 0), Vector(0.1, 1, 0), 0},
        {2, Point(0, 3, -5), Vector(0, 0, 1), 0},
        {3, Point(0, 0, -5), Vector(0, 0, 1), 0},
        {4, Point(0, 2, -5), Vector(0, 0, 1), 0},
        {5, Point(0, 1, -5), Vector(0, 0, 1), 0},
        {6, Point(0, 1.5, -2), Vector(0, 0, 1), 2},
    }));
    GIVEN("A cylinder constrained to 1 < y < 2 and ray " + std::to_string(row)) {
        Cylinder cyl{};
        cyl.minimum = 1;
        cyl.maximum = 2;
        const Ray r{point, Vector::normalize(direction)};
        WHEN("The ray is intersected with the cylinder") {
            const auto xs = local_intersect(cyl, r);
            THEN("The ray only hits the cylinder between its minimum and maximum") {
                REQUIRE(xs.size() == count);
            }
        }
    }
}

SCENARIO("The default closed value for a cylinder") {
    GIVEN("A default cylinder") {
        const Cylinder cyl{};
        THEN("It is open at both ends") {
            REQUIRE_FALSE(cyl.closed);
        }
    }
}

SCENARIO("Intersecting the caps of a closed cylinder") {
    const auto [row, point, direction, count] = GENERATE(table<int, Point, Vector, std::size_t>({
        {1, Point(0, 3, 0), Vector(0, -1, 0), 2},
        {2, Point(0, 3, -2), Vector(0, -1, 2), 2},
        {3, Point(0, 4, -2), Vector(0, -1, 1), 2}, // corner case
        {4, Point(0, 0, -2), Vector(0, 1, 2), 2},
        {5, Point(0, -1, -2), Vector(0, 1, 1), 2}, // corner case
    }));
    GIVEN("A closed cylinder constrained to 1 < y < 2 and ray " + std::to_string(row)) {
        Cylinder cyl{};
        cyl.minimum = 1;
        cyl.maximum = 2;
        cyl.closed = true;
        const Ray r{point, Vector::normalize(direction)};
        WHEN("The ray is intersected with the cylinder") {
            const auto xs = local_intersect(cyl, r);
            THEN("The caps are counted as intersections") {
                REQUIRE(xs.size() == count);
            }
        }
    }
}

SCENARIO("The normal vector on a cylinder's end caps") {
    const auto [point, expected_normal] = GENERATE(table<Point, Vector>({
        {Point(0, 1, 0), Vector(0, -1, 0)},
        {Point(0.5, 1, 0), Vector(0, -1, 0)},
        {Point(0, 1, 0.5), Vector(0, -1, 0)},
        {Point(0, 2, 0), Vector(0, 1, 0)},
        {Point(0.5, 2, 0), Vector(0, 1, 0)},
        {Point(0, 2, 0.5), Vector(0, 1, 0)},
    }));
    GIVEN("A closed cylinder constrained to 1 < y < 2 and a point on one of its caps") {
        Cylinder cyl{};
        cyl.minimum = 1;
        cyl.maximum = 2;
        cyl.closed = true;
        WHEN("The normal is computed at the point") {
            const auto n = local_normal_at(cyl, point);
            THEN("The normal points straight out of the cap along y") {
                REQUIRE(Vector::areAlmostEqual(n, expected_normal));
            }
        }
    }
}
