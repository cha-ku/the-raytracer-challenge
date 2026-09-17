#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

SCENARIO("The normal of a plane is constant everywhere") {
    GIVEN("Plane") {
        const Plane p = Plane::make_plane();
        WHEN("Normal is computed at several points") {
            const auto n1 = local_normal_at(p, Point(0, 0, 0));
            const auto n2 = local_normal_at(p, Point(10, 0, -10));
            const auto n3 = local_normal_at(p, Point(-5, 0, 150));
            THEN("Normal is always vector(0, 1, 0)") {
                REQUIRE(Vector::areAlmostEqual(n1, Vector(0, 1, 0)));
                REQUIRE(Vector::areAlmostEqual(n2, Vector(0, 1, 0)));
                REQUIRE(Vector::areAlmostEqual(n3, Vector(0, 1, 0)));
            }
        }
    }
}
