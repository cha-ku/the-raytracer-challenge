#include "Intersect.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

namespace {
    bool points_equal(const Point &a, const Point &b) {
        return utils::equal(a.x, b.x) && utils::equal(a.y, b.y) && utils::equal(a.z, b.z);
    }

    // The feature's Background: tri <- smooth_triangle(p1, p2, p3, n1, n2, n3)
    SmoothTriangle background_triangle() {
        return SmoothTriangle{
            Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0),
            Vector(0, 1, 0), Vector(-1, 0, 0), Vector(1, 0, 0)
        };
    }
}

SCENARIO (
"Constructing a smooth triangle"
)
 {
    // Background shared by the smooth-triangle scenarios
    GIVEN("Three corner points and a normal at each corner") {
        const Point p1{0, 1, 0};
        const Point p2{-1, 0, 0};
        const Point p3{1, 0, 0};
        const Vector n1{0, 1, 0};
        const Vector n2{-1, 0, 0};
        const Vector n3{1, 0, 0};
        WHEN("A smooth triangle is constructed from them") {
            const SmoothTriangle tri{p1, p2, p3, n1, n2, n3};
            THEN("It stores its corners and their normals") {
                REQUIRE(points_equal(tri.p1, p1));
                REQUIRE(points_equal(tri.p2, p2));
                REQUIRE(points_equal(tri.p3, p3));
                REQUIRE(Vector::areAlmostEqual(tri.n1, n1));
                REQUIRE(Vector::areAlmostEqual(tri.n2, n2));
                REQUIRE(Vector::areAlmostEqual(tri.n3, n3));
            }
        }
    }
}

SCENARIO (
"An intersection with a smooth triangle stores u/v"
)
 {
    GIVEN("A smooth triangle and a ray aimed inside it") {
        const SmoothTriangle tri{background_triangle()};
        const Ray r{Point(-0.2, 0.3, -2), Vector(0, 0, 1)};
        WHEN("The ray is intersected with the triangle in object space") {
            const auto hit = local_intersect(tri, r);
            THEN("The hit stores the barycentric u and v") {
                REQUIRE(hit.has_value());
                REQUIRE(utils::equal(hit->u, 0.45f));
                REQUIRE(utils::equal(hit->v, 0.25f));
            }
        }
        WHEN("The ray is intersected with a Geom holding the triangle") {
            const Geom geom{tri};
            const auto xs = intersect(geom, r);
            THEN("The Intersection carries the same u and v") {
                REQUIRE(xs.size() == 1);
                REQUIRE(xs[0].u.has_value());
                REQUIRE(xs[0].v.has_value());
                REQUIRE(utils::equal(*xs[0].u, 0.45f));
                REQUIRE(utils::equal(*xs[0].v, 0.25f));
            }
        }
    }
}

SCENARIO (
"A smooth triangle uses u/v to interpolate the normal"
)
 {
    GIVEN("A smooth triangle") {
        const Geom tri{background_triangle()};
        WHEN("An intersection with u = 0.45, v = 0.25 is used to find the normal") {
            // The book's intersection_with_uv(1, tri, 0.45, 0.25)
            const Intersection i{.object = &tri, .t = 1, .u = 0.45f, .v = 0.25f};
            const Vector n = normal_at(tri, Point(0, 0, 0), i);
            THEN("The normal is blended from the corner normals, not the flat face normal") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(-0.5547, 0.83205, 0)));
            }
        }
    }
}

SCENARIO (
"Preparing the normal on a smooth triangle"
)
 {
    GIVEN("A smooth triangle") {
        const Geom tri{background_triangle()};
        WHEN("The computations are prepared for a hit with u = 0.45, v = 0.25") {
            const Intersection i{.object = &tri, .t = 1, .u = 0.45f, .v = 0.25f};
            const Ray r{Point(-0.2, 0.3, -2), Vector(0, 0, 1)};
            const auto xs = intersections(i);
            const Computations comps = prepare_computations(i, r, xs);
            THEN("The normal vector is the interpolated one") {
                REQUIRE(Vector::areAlmostEqual(comps.normal_vector, Vector(-0.5547, 0.83205, 0)));
            }
        }
    }
}
