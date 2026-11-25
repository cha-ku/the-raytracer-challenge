#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "TestShape.hpp"

#include "catch2/catch_test_macros.hpp"

#include <algorithm>
#include <memory>
#include <variant>
#include <vector>

using namespace raytracer;

SCENARIO (
"Adding a child to a group"
)
 {
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

SCENARIO (
"Intersecting a ray with an empty group"
)
 {
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

SCENARIO (
"Intersecting a ray with a nonempty group"
)
 {
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

SCENARIO (
"Intersecting a transformed group"
)
 {
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

SCENARIO("Partitioning a group's children") {
    GIVEN("A group of three spheres: one on the left, one on the right and one across the middle") {
        Geom s1 = make_sphere();
        s1.set_transform(translation<double>(-2, 0, 0));
        Geom s2 = make_sphere();
        s2.set_transform(translation<double>(2, 0, 0));
        Geom s3 = make_sphere();
        Geom g = make_group();
        add_child(g, s1);
        add_child(g, s2);
        add_child(g, s3);
        WHEN("The group's children are partitioned") {
            const auto [left, right] = partition_children(g);
            THEN("The group keeps only the sphere that fits in neither half") {
                REQUIRE(std::get<Group>(g.shape).children == std::vector<Geom *>{&s3});
                REQUIRE(left == std::vector<Geom *>{&s1});
                REQUIRE(right == std::vector<Geom *>{&s2});
            }
        }
    }
}

SCENARIO("Creating a sub-group from a list of children") {
    GIVEN("An empty group and two spheres") {
        Geom s1 = make_sphere();
        Geom s2 = make_sphere();
        Geom g = make_group();
        // Owns the new sub-group, as World::objects or ObjParser::geoms would
        std::vector<std::unique_ptr<Geom>> geoms;
        WHEN("A sub-group is made from the spheres") {
            make_subgroup(geoms, g, {&s1, &s2});
            THEN("The group's only child is a group of both spheres") {
                const auto &children = std::get<Group>(g.shape).children;
                REQUIRE(children.size() == 1);
                const Geom *subgroup = children[0];
                REQUIRE(std::get<Group>(subgroup->shape).children == std::vector<Geom *>{&s1, &s2});
                REQUIRE(subgroup->parent == &g);
                REQUIRE(s1.parent == subgroup);
                REQUIRE(s2.parent == subgroup);
            }
        }
    }
}

SCENARIO("Subdividing a group partitions its children") {
    GIVEN("A group of two small spheres on the left, one above the other, and a large sphere around the origin") {
        Geom s1 = make_sphere();
        s1.set_transform(translation<double>(-2, -2, 0));
        Geom s2 = make_sphere();
        s2.set_transform(translation<double>(-2, 2, 0));
        Geom s3 = make_sphere();
        s3.set_transform(scale<double>(4, 4, 4));
        Geom g = make_group();
        add_child(g, s1);
        add_child(g, s2);
        add_child(g, s3);
        // Owns the new sub-groups
        std::vector<std::unique_ptr<Geom>> geoms;
        WHEN("The group is divided") {
            divide(geoms, g, 1);
            THEN("The large sphere stays, and the small ones move to a sub-group that is itself divided") {
                const auto &children = std::get<Group>(g.shape).children;
                REQUIRE(children.size() == 2);
                REQUIRE(children[0] == &s3);
                const auto &subgroup = std::get<Group>(children[1]->shape);
                REQUIRE(subgroup.children.size() == 2);
                REQUIRE(std::get<Group>(subgroup.children[0]->shape).children == std::vector<Geom *>{&s1});
                REQUIRE(std::get<Group>(subgroup.children[1]->shape).children == std::vector<Geom *>{&s2});
            }
        }
    }
}

SCENARIO("Subdividing a group with too few children") {
    GIVEN("A group of two: a sub-group of three spheres, and a sphere") {
        Geom s1 = make_sphere();
        s1.set_transform(translation<double>(-2, 0, 0));
        Geom s2 = make_sphere();
        s2.set_transform(translation<double>(2, 1, 0));
        Geom s3 = make_sphere();
        s3.set_transform(translation<double>(2, -1, 0));
        Geom subgroup = make_group();
        add_child(subgroup, s1);
        add_child(subgroup, s2);
        add_child(subgroup, s3);
        Geom s4 = make_sphere();
        Geom g = make_group();
        add_child(g, subgroup);
        add_child(g, s4);
        // Owns the new sub-groups
        std::vector<std::unique_ptr<Geom>> geoms;
        WHEN("The group is divided with a threshold of 3") {
            divide(geoms, g, 3);
            THEN("The group itself is unchanged, but its sub-group is divided") {
                REQUIRE(std::get<Group>(g.shape).children == std::vector<Geom *>{&subgroup, &s4});
                const auto &children = std::get<Group>(subgroup.shape).children;
                REQUIRE(children.size() == 2);
                REQUIRE(std::get<Group>(children[0]->shape).children == std::vector<Geom *>{&s1});
                REQUIRE(std::get<Group>(children[1]->shape).children == std::vector<Geom *>{&s2, &s3});
            }
        }
    }
}
