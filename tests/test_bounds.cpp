#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <limits>
#include <numbers>

using namespace raytracer;

namespace {
    bool points_equal(const Point &a, const Point &b) {
        return utils::equal(a.x, b.x) && utils::equal(a.y, b.y) && utils::equal(a.z, b.z);
    }
}

SCENARIO("Creating an empty bounding box") {
    GIVEN("A default-constructed box") {
        constexpr float infinity{std::numeric_limits<float>::infinity()};
        const BoundingBox box{};
        THEN("It is empty, with min at infinity and max at -infinity") {
            REQUIRE(box.empty());
            // Compared exactly: points_equal subtracts, and infinity - infinity is NaN
            REQUIRE(box.min == Point(infinity, infinity, infinity));
            REQUIRE(box.max == Point(-infinity, -infinity, -infinity));
        }
    }
}

SCENARIO("Creating a bounding box with volume") {
    GIVEN("A box created from two corners") {
        const BoundingBox box{.min = Point(-1, -2, -3), .max = Point(3, 2, 1)};
        THEN("It holds both corners") {
            REQUIRE(points_equal(box.min, Point(-1, -2, -3)));
            REQUIRE(points_equal(box.max, Point(3, 2, 1)));
        }
    }
}

SCENARIO("Adding points to an empty bounding box") {
    GIVEN("An empty box and two points") {
        BoundingBox box{};
        const Point p1(-5, 2, 0);
        const Point p2(7, 0, -3);
        WHEN("Both points are added to the box") {
            box.add(p1);
            box.add(p2);
            THEN("The box just contains both points") {
                REQUIRE(points_equal(box.min, Point(-5, 0, -3)));
                REQUIRE(points_equal(box.max, Point(7, 2, 0)));
            }
        }
    }
}

SCENARIO("Adding one bounding box to another") {
    GIVEN("Two boxes") {
        BoundingBox box1{.min = Point(-5, -2, 0), .max = Point(7, 4, 4)};
        const BoundingBox box2{.min = Point(8, -7, -2), .max = Point(14, 2, 8)};
        WHEN("The second is added to the first") {
            box1.add(box2);
            THEN("The first just contains both boxes") {
                REQUIRE(points_equal(box1.min, Point(-5, -7, -2)));
                REQUIRE(points_equal(box1.max, Point(14, 4, 8)));
            }
        }
    }
}

SCENARIO("Checking to see if a box contains a given point") {
    const auto [point, expected] = GENERATE(table<Point, bool>({
        {Point(5, -2, 0), true},
        {Point(11, 4, 7), true},
        {Point(8, 1, 3), true},
        {Point(3, 0, 3), false},
        {Point(8, -4, 3), false},
        {Point(8, 1, -1), false},
        {Point(13, 1, 3), false},
        {Point(8, 5, 3), false},
        {Point(8, 1, 8), false},
    }));
    GIVEN("A box and a point") {
        CAPTURE(point.x, point.y, point.z);
        const BoundingBox box{.min = Point(5, -2, 0), .max = Point(11, 4, 7)};
        THEN("The box contains the point if it is inside or on the surface") {
            REQUIRE(box.contains(point) == expected);
        }
    }
}

SCENARIO("Checking to see if a box contains a given box") {
    const auto [min, max, expected] = GENERATE(table<Point, Point, bool>({
        {Point(5, -2, 0), Point(11, 4, 7), true},
        {Point(6, -1, 1), Point(10, 3, 6), true},
        {Point(4, -3, -1), Point(10, 3, 6), false},
        {Point(6, -1, 1), Point(12, 5, 8), false},
    }));
    GIVEN("Two boxes") {
        CAPTURE(min.x, min.y, min.z, max.x, max.y, max.z);
        const BoundingBox box{.min = Point(5, -2, 0), .max = Point(11, 4, 7)};
        const BoundingBox box2{.min = min, .max = max};
        THEN("The first contains the second only if all of it is inside") {
            REQUIRE(box.contains(box2) == expected);
        }
    }
}

SCENARIO("Transforming a bounding box") {
    GIVEN("A box and a rotation about two axes") {
        constexpr double pi{std::numbers::pi};
        const BoundingBox box{.min = Point(-1, -1, -1), .max = Point(1, 1, 1)};
        const auto matrix = multiply(rotation_x(pi / 4), rotation_y(pi / 4));
        WHEN("The box is transformed") {
            const BoundingBox box2 = transform(box, matrix);
            THEN("The result is the axis-aligned box around the rotated corners") {
                REQUIRE(points_equal(box2.min, Point(-1.4142, -1.7071, -1.7071)));
                REQUIRE(points_equal(box2.max, Point(1.4142, 1.7071, 1.7071)));
            }
        }
    }
}

SCENARIO("Intersecting a ray with a bounding box at the origin") {
    const auto [origin, direction, expected] = GENERATE(table<Point, Vector, bool>({
        {Point(5, 0.5, 0), Vector(-1, 0, 0), true},
        {Point(-5, 0.5, 0), Vector(1, 0, 0), true},
        {Point(0.5, 5, 0), Vector(0, -1, 0), true},
        {Point(0.5, -5, 0), Vector(0, 1, 0), true},
        {Point(0.5, 0, 5), Vector(0, 0, -1), true},
        {Point(0.5, 0, -5), Vector(0, 0, 1), true},
        {Point(0, 0.5, 0), Vector(0, 0, 1), true},
        {Point(-2, 0, 0), Vector(2, 4, 6), false},
        {Point(0, -2, 0), Vector(6, 2, 4), false},
        {Point(0, 0, -2), Vector(4, 6, 2), false},
        {Point(2, 0, 2), Vector(0, 0, -1), false},
        {Point(0, 2, 2), Vector(0, -1, 0), false},
        {Point(2, 2, 0), Vector(-1, 0, 0), false},
    }));
    GIVEN("A cubic box at the origin and a ray") {
        CAPTURE(origin.x, origin.y, origin.z, direction.x, direction.y, direction.z);
        const BoundingBox box{.min = Point(-1, -1, -1), .max = Point(1, 1, 1)};
        const Ray r{origin, Vector::normalize(direction)};
        THEN("The ray hits or misses the box as expected") {
            REQUIRE(intersects(box, r) == expected);
        }
    }
}

SCENARIO("Intersecting a ray with a non-cubic bounding box") {
    const auto [origin, direction, expected] = GENERATE(table<Point, Vector, bool>({
        {Point(15, 1, 2), Vector(-1, 0, 0), true},
        {Point(-5, -1, 4), Vector(1, 0, 0), true},
        {Point(7, 6, 5), Vector(0, -1, 0), true},
        {Point(9, -5, 6), Vector(0, 1, 0), true},
        {Point(8, 2, 12), Vector(0, 0, -1), true},
        {Point(6, 0, -5), Vector(0, 0, 1), true},
        {Point(8, 1, 3.5), Vector(0, 0, 1), true},
        {Point(9, -1, -8), Vector(2, 4, 6), false},
        {Point(8, 3, -4), Vector(6, 2, 4), false},
        {Point(9, -1, -2), Vector(4, 6, 2), false},
        {Point(4, 0, 9), Vector(0, 0, -1), false},
        {Point(8, 6, -1), Vector(0, -1, 0), false},
        {Point(12, 5, 4), Vector(-1, 0, 0), false},
    }));
    GIVEN("A non-cubic box away from the origin and a ray") {
        CAPTURE(origin.x, origin.y, origin.z, direction.x, direction.y, direction.z);
        const BoundingBox box{.min = Point(5, -2, 0), .max = Point(11, 4, 7)};
        const Ray r{origin, Vector::normalize(direction)};
        THEN("The ray hits or misses the box as expected") {
            REQUIRE(intersects(box, r) == expected);
        }
    }
}

SCENARIO("Splitting a perfect cube") {
    GIVEN("A box with equal sides") {
        const BoundingBox box{.min = Point(-1, -4, -5), .max = Point(9, 6, 5)};
        WHEN("The box is split") {
            const auto [left, right] = split_bounds(box);
            THEN("It is halved along x") {
                REQUIRE(points_equal(left.min, Point(-1, -4, -5)));
                REQUIRE(points_equal(left.max, Point(4, 6, 5)));
                REQUIRE(points_equal(right.min, Point(4, -4, -5)));
                REQUIRE(points_equal(right.max, Point(9, 6, 5)));
            }
        }
    }
}

SCENARIO("Splitting an x-wide box") {
    GIVEN("A box that is widest along x") {
        const BoundingBox box{.min = Point(-1, -2, -3), .max = Point(9, 5.5, 3)};
        WHEN("The box is split") {
            const auto [left, right] = split_bounds(box);
            THEN("It is halved along x") {
                REQUIRE(points_equal(left.min, Point(-1, -2, -3)));
                REQUIRE(points_equal(left.max, Point(4, 5.5, 3)));
                REQUIRE(points_equal(right.min, Point(4, -2, -3)));
                REQUIRE(points_equal(right.max, Point(9, 5.5, 3)));
            }
        }
    }
}

SCENARIO("Splitting a y-wide box") {
    GIVEN("A box that is widest along y") {
        const BoundingBox box{.min = Point(-1, -2, -3), .max = Point(5, 8, 3)};
        WHEN("The box is split") {
            const auto [left, right] = split_bounds(box);
            THEN("It is halved along y") {
                REQUIRE(points_equal(left.min, Point(-1, -2, -3)));
                REQUIRE(points_equal(left.max, Point(5, 3, 3)));
                REQUIRE(points_equal(right.min, Point(-1, 3, -3)));
                REQUIRE(points_equal(right.max, Point(5, 8, 3)));
            }
        }
    }
}

SCENARIO("Splitting a z-wide box") {
    GIVEN("A box that is widest along z") {
        const BoundingBox box{.min = Point(-1, -2, -3), .max = Point(5, 3, 7)};
        WHEN("The box is split") {
            const auto [left, right] = split_bounds(box);
            THEN("It is halved along z") {
                REQUIRE(points_equal(left.min, Point(-1, -2, -3)));
                REQUIRE(points_equal(left.max, Point(5, 3, 2)));
                REQUIRE(points_equal(right.min, Point(-1, -2, 2)));
                REQUIRE(points_equal(right.max, Point(5, 3, 7)));
            }
        }
    }
}
