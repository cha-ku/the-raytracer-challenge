#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "TestShape.hpp"

#include "catch2/catch_test_macros.hpp"

#include <algorithm>

using namespace raytracer;

SCENARIO("Adding a child to a group") {
    GIVEN("A group and a test shape") {
        Geom g = make_group();
        Geom s = make_test_shape();
        WHEN("The shape is added as a child of the group") {
            add_child(g, s);
            THEN("The group is not empty, includes the shape, and is the shape's parent") {
                const auto &children = std::get<Group>(g.shape).children;
                REQUIRE_FALSE(children.empty());
                REQUIRE(std::ranges::find(children, &s) != children.end());
                REQUIRE(s.parent == &g);
            }
        }
    }
}

SCENARIO("Intersecting a ray with an empty group") {
    GIVEN("An empty group and a ray") {
        const Geom g = make_group();
        const Ray r{Point(0, 0, 0), Vector(0, 0, 1)};
        WHEN("The ray is intersected with the group") {
            // The group has the identity transform, so intersect() is the same as the book's local_intersect()
            const auto xs = intersect(g, r);
            THEN("There are no intersections") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("Intersecting a ray with a nonempty group") {
    GIVEN("A group with three spheres") {
        Geom g = make_group();
        Geom s1 = make_sphere();
        Geom s2 = make_sphere();
        s2.set_transform(translation<double>(0, 0, -3));
        Geom s3 = make_sphere();
        s3.set_transform(translation<double>(5, 0, 0));
        add_child(g, s1);
        add_child(g, s2);
        add_child(g, s3);
        WHEN("A ray is intersected with the group") {
            const Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
            // Identity group transform: intersect() is the same as the book's local_intersect()
            const auto xs = intersect(g, r);
            THEN("The ray hits s2 then s1, sorted by t, and misses s3") {
                REQUIRE(xs.size() == 4);
                REQUIRE(xs[0].object == &s2);
                REQUIRE(xs[1].object == &s2);
                REQUIRE(xs[2].object == &s1);
                REQUIRE(xs[3].object == &s1);
            }
        }
    }
}

SCENARIO("Intersecting a transformed group") {
    GIVEN("A scaled group with a translated sphere") {
        Geom g = make_group();
        g.set_transform(scale<double>(2, 2, 2));
        Geom s = make_sphere();
        s.set_transform(translation<double>(5, 0, 0));
        add_child(g, s);
        WHEN("A ray is intersected with the group") {
            const Ray r{Point(10, 0, -10), Vector(0, 0, 1)};
            const auto xs = intersect(g, r);
            THEN("The ray hits the sphere twice") {
                REQUIRE(xs.size() == 2);
            }
        }
    }
}
