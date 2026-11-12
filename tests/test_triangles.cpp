#include "Intersect.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

namespace {
    bool points_equal(const Point &a, const Point &b) {
        return utils::equal(a.x, b.x) && utils::equal(a.y, b.y) && utils::equal(a.z, b.z);
    }
}

SCENARIO("Constructing a triangle") {
    GIVEN("Three points") {
        const Point p1{0, 1, 0};
        const Point p2{-1, 0, 0};
        const Point p3{1, 0, 0};
        WHEN("A triangle is constructed from them") {
            const Triangle t{p1, p2, p3};
            THEN("It stores its corners, edge vectors and normal") {
                REQUIRE(points_equal(t.v1, p1));
                REQUIRE(points_equal(t.v2, p2));
                REQUIRE(points_equal(t.v3, p3));
                REQUIRE(Vector::areAlmostEqual(t.e1, Vector(-1, -1, 0)));
                REQUIRE(Vector::areAlmostEqual(t.e2, Vector(1, -1, 0)));
                REQUIRE(Vector::areAlmostEqual(t.normal, Vector(0, 0, -1)));
            }
        }
    }
}

SCENARIO("Finding the normal on a triangle") {
    GIVEN("A triangle") {
        const Triangle t{Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)};
        WHEN("The normal is computed at three points on it") {
            const auto n1 = local_normal_at(t, Point(0, 0.5, 0));
            const auto n2 = local_normal_at(t, Point(-0.5, 0.75, 0));
            const auto n3 = local_normal_at(t, Point(0.5, 0.25, 0));
            THEN("Every normal is the triangle's precomputed normal") {
                REQUIRE(Vector::areAlmostEqual(n1, t.normal));
                REQUIRE(Vector::areAlmostEqual(n2, t.normal));
                REQUIRE(Vector::areAlmostEqual(n3, t.normal));
            }
        }
    }
}

SCENARIO("Intersecting a ray parallel to the triangle") {
    GIVEN("A triangle and a ray parallel to its plane") {
        const Triangle t{Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)};
        const Ray r{Point(0, -1, -2), Vector(0, 1, 0)};
        WHEN("The ray is intersected with the triangle") {
            const auto xs = local_intersect(t, r);
            THEN("There are no intersections") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("A ray misses the p1-p3 edge") {
    GIVEN("A triangle and a ray passing outside its p1-p3 edge") {
        const Triangle t{Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)};
        const Ray r{Point(1, 1, -2), Vector(0, 0, 1)};
        WHEN("The ray is intersected with the triangle") {
            const auto xs = local_intersect(t, r);
            THEN("There are no intersections") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("A ray misses the p1-p2 edge") {
    GIVEN("A triangle and a ray passing outside its p1-p2 edge") {
        const Triangle t{Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)};
        const Ray r{Point(-1, 1, -2), Vector(0, 0, 1)};
        WHEN("The ray is intersected with the triangle") {
            const auto xs = local_intersect(t, r);
            THEN("There are no intersections") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("A ray misses the p2-p3 edge") {
    GIVEN("A triangle and a ray passing outside its p2-p3 edge") {
        const Triangle t{Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)};
        const Ray r{Point(0, -1, -2), Vector(0, 0, 1)};
        WHEN("The ray is intersected with the triangle") {
            const auto xs = local_intersect(t, r);
            THEN("There are no intersections") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("A ray strikes a triangle") {
    GIVEN("A triangle and a ray aimed inside it") {
        const Triangle t{Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)};
        const Ray r{Point(0, 0.5, -2), Vector(0, 0, 1)};
        WHEN("The ray is intersected with the triangle") {
            const auto xs = local_intersect(t, r);
            THEN("There is one intersection at t = 2") {
                REQUIRE(xs.size() == 1);
                REQUIRE(utils::equal(xs[0], 2.0f));
            }
        }
    }
}
