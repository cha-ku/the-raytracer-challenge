#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <cstddef>
#include <memory>
#include <variant>
#include <vector>

using namespace raytracer;

SCENARIO("CSG is created with an operation and two shapes") {
    GIVEN("A sphere and a cube") {
        Geom s1 = make_sphere();
        Geom s2 = make_cube();
        WHEN("A CSG is created from their union") {
            const Geom c = make_csg(Csg::Operation::Union, s1, s2);
            THEN("It stores the operation and both shapes, and is the parent of both") {
                const auto &csg = std::get<Csg>(c.shape);
                REQUIRE(csg.op == Csg::Operation::Union);
                REQUIRE(csg.left == &s1);
                REQUIRE(csg.right == &s2);
                REQUIRE(s1.parent == &c);
                REQUIRE(s2.parent == &c);
            }
        }
    }
}

SCENARIO("Evaluating the rule for a CSG operation") {
    using enum Csg::Operation;
    // lhit: was the left shape hit; inl/inr: is the hit inside the left/right shape
    const auto [op, lhit, inl, inr, expected] = GENERATE(table<Csg::Operation, bool, bool, bool, bool>({
        {Union, true, true, true, false},
        {Union, true, true, false, true},
        {Union, true, false, true, false},
        {Union, true, false, false, true},
        {Union, false, true, true, false},
        {Union, false, true, false, false},
        {Union, false, false, true, true},
        {Union, false, false, false, true},
        {Intersection, true, true, true, true},
        {Intersection, true, true, false, false},
        {Intersection, true, false, true, true},
        {Intersection, true, false, false, false},
        {Intersection, false, true, true, true},
        {Intersection, false, true, false, true},
        {Intersection, false, false, true, false},
        {Intersection, false, false, false, false},
        {Difference, true, true, true, false},
        {Difference, true, true, false, true},
        {Difference, true, false, true, false},
        {Difference, true, false, false, true},
        {Difference, false, true, true, true},
        {Difference, false, true, false, true},
        {Difference, false, false, true, false},
        {Difference, false, false, false, false},
    }));
    GIVEN("A CSG operation and a hit") {
        CAPTURE(static_cast<int>(op), lhit, inl, inr);
        WHEN("The rule is evaluated") {
            const bool result = intersection_allowed(op, lhit, inl, inr);
            THEN("The hit is kept only if the operation allows it") {
                REQUIRE(result == expected);
            }
        }
    }
}

SCENARIO("Filtering a list of intersections") {
    using enum Csg::Operation;
    // x0, x1: indices into xs of the two intersections that are kept
    const auto [op, x0, x1] = GENERATE(table<Csg::Operation, std::size_t, std::size_t>({
        {Union, 0, 3},
        {Intersection, 1, 2},
        {Difference, 0, 1},
    }));
    GIVEN("A CSG of a sphere and a cube, and a list of intersections with both") {
        CAPTURE(static_cast<int>(op), x0, x1);
        Geom s1 = make_sphere();
        Geom s2 = make_cube();
        const Geom c = make_csg(op, s1, s2);
        const std::vector<raytracer::Intersection> xs{
            {.object = &s1, .t = 1},
            {.object = &s2, .t = 2},
            {.object = &s1, .t = 3},
            {.object = &s2, .t = 4},
        };
        WHEN("The intersections are filtered") {
            const auto result = filter_intersections(std::get<Csg>(c.shape), xs);
            THEN("Only the two allowed by the operation are kept, in order") {
                REQUIRE(result.size() == 2);
                REQUIRE(result[0] == xs[x0]);
                REQUIRE(result[1] == xs[x1]);
            }
        }
    }
}

SCENARIO("A ray misses a CSG object") {
    GIVEN("The union of a sphere and a cube, and a ray that passes above both") {
        Geom s1 = make_sphere();
        Geom s2 = make_cube();
        const Geom c = make_csg(Csg::Operation::Union, s1, s2);
        const Ray r{Point(0, 2, -5), Vector(0, 0, 1)};
        WHEN("The ray is intersected with the CSG") {
            const auto xs = local_intersect(std::get<Csg>(c.shape), r);
            THEN("There are no intersections") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("A ray hits a CSG object") {
    GIVEN("The union of two overlapping spheres, and a ray through both") {
        Geom s1 = make_sphere();
        Geom s2 = make_sphere();
        s2.set_transform(translation(0.0, 0.0, 0.5));
        const Geom c = make_csg(Csg::Operation::Union, s1, s2);
        const Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
        WHEN("The ray is intersected with the CSG") {
            const auto xs = local_intersect(std::get<Csg>(c.shape), r);
            THEN("It enters through the first sphere and leaves through the second") {
                REQUIRE(xs.size() == 2);
                REQUIRE(utils::equal(xs[0].t, 4));
                REQUIRE(xs[0].object == &s1);
                REQUIRE(utils::equal(xs[1].t, 6.5));
                REQUIRE(xs[1].object == &s2);
            }
        }
    }
}

SCENARIO("Subdividing a CSG shape subdivides its children") {
    GIVEN("The difference of two groups, each of two spheres that are apart") {
        Geom s1 = make_sphere();
        s1.set_transform(translation(-1.5, 0.0, 0.0));
        Geom s2 = make_sphere();
        s2.set_transform(translation(1.5, 0.0, 0.0));
        Geom left = make_group();
        add_child(left, s1);
        add_child(left, s2);
        Geom s3 = make_sphere();
        s3.set_transform(translation(0.0, 0.0, -1.5));
        Geom s4 = make_sphere();
        s4.set_transform(translation(0.0, 0.0, 1.5));
        Geom right = make_group();
        add_child(right, s3);
        add_child(right, s4);
        Geom shape = make_csg(Csg::Operation::Difference, left, right);
        // Owns the new sub-groups
        std::vector<std::unique_ptr<Geom>> geoms;
        WHEN("The CSG is divided") {
            divide(geoms, shape, 1);
            THEN("Both groups are divided") {
                const auto &left_children = std::get<Group>(left.shape).children;
                REQUIRE(left_children.size() == 2);
                REQUIRE(std::get<Group>(left_children[0]->shape).children == std::vector<Geom *>{&s1});
                REQUIRE(std::get<Group>(left_children[1]->shape).children == std::vector<Geom *>{&s2});
                const auto &right_children = std::get<Group>(right.shape).children;
                REQUIRE(right_children.size() == 2);
                REQUIRE(std::get<Group>(right_children[0]->shape).children == std::vector<Geom *>{&s3});
                REQUIRE(std::get<Group>(right_children[1]->shape).children == std::vector<Geom *>{&s4});
            }
        }
    }
}
