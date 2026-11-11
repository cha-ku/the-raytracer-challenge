#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "TestShape.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"
#include <cmath>
#include <numbers>

using namespace raytracer;

SCENARIO("The default transformation") {
    GIVEN("Shape") {
        const auto s = make_test_shape();
        THEN("Transform is identity matrix") {
            REQUIRE(s.transform() == Matrix<double>::identity(4));
        }
    }
}

SCENARIO("Assigning a transformation") {
    GIVEN("Shape and translation") {
        auto s = make_test_shape();
        const auto t = translation<double>(2, 3, 4);
        WHEN("set_transform is called") {
            s.set_transform(t);
            THEN("Transform equals the translation") {
                REQUIRE(s.transform() == t);
            }
        }
    }
}

SCENARIO("A shape's default material") {
    GIVEN("Shape") {
        const auto s = make_test_shape();
        WHEN("Material is retrieved") {
            const auto m = s.material;
            THEN("Material equals the default material") {
                REQUIRE(m == Material{});
            }
        }
    }
}

SCENARIO("Assigning a material to a shape") {
    GIVEN("Shape and material") {
        auto s = make_test_shape();
        Material m{};
        m.ambient = 1;
        WHEN("Material is assigned to shape") {
            s.material = m;
            THEN("Shape has the assigned material") {
                REQUIRE(s.material == m);
            }
        }
    }
}

SCENARIO("Intersecting a scaled shape with a ray") {
    GIVEN("Ray and shape") {
        const Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
        auto s = make_test_shape();
        WHEN("Shape is scaled and intersected") {
            s.set_transform(scale<double>(2, 2, 2));
            const auto xs = intersect(s, r);
            THEN("Saved ray is in the shape's object space") {
                REQUIRE(saved_ray(s).origin == Point(0, 0, -2.5));
                REQUIRE(saved_ray(s).direction == Vector(0, 0, 0.5));
            }
        }
    }
}

SCENARIO("Intersecting a translated shape with a ray") {
    GIVEN("Ray and shape") {
        const Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
        auto s = make_test_shape();
        WHEN("Shape is translated and intersected") {
            s.set_transform(translation<double>(5, 0, 0));
            const auto xs = intersect(s, r);
            THEN("Saved ray is in the shape's object space") {
                REQUIRE(saved_ray(s).origin == Point(-5, 0, -5));
                REQUIRE(saved_ray(s).direction == Vector(0, 0, 1));
            }
        }
    }
}

SCENARIO("Computing the normal on a translated shape") {
    GIVEN("Shape with translation") {
        auto s = make_test_shape();
        s.set_transform(translation<double>(0, 1, 0));
        WHEN("Normal is computed") {
            const auto n = normal_at(s, Point(0, 1.70711, -0.70711));
            THEN("Normal accounts for translation") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(0, 0.707107, -0.707107)));
            }
        }
    }
}

SCENARIO("Computing the normal on a transformed shape") {
    GIVEN("Shape with scaling and rotation") {
        auto s = make_test_shape();
        const auto m = multiply(scale<double>(1, 0.5, 1), rotation_z(std::numbers::pi / 5));
        s.set_transform(m);
        WHEN("Normal is computed") {
            const auto sqrt2_over_2 = std::sqrt(2) / 2;
            const auto n = normal_at(s, Point(0, sqrt2_over_2, -sqrt2_over_2));
            THEN("Normal accounts for transformation") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(0, 0.970143, -0.242536)));
            }
        }
    }
}

SCENARIO("A shape has a parent attribute") {
    GIVEN("A test shape") {
        const auto s = make_test_shape();
        THEN("It has no parent") {
            REQUIRE(s.parent == nullptr);
        }
    }
}

SCENARIO("Converting a point from world to object space") {
    GIVEN("A sphere inside a scaled group inside a rotated group") {
        Geom g1 = make_group();
        g1.set_transform(rotation_y(std::numbers::pi / 2));
        Geom g2 = make_group();
        g2.set_transform(scale<double>(2, 2, 2));
        add_child(g1, g2);
        Geom s = make_sphere();
        s.set_transform(translation<double>(5, 0, 0));
        add_child(g2, s);
        WHEN("A world-space point is converted into the sphere's object space") {
            const auto p = world_to_object(s, Point(-2, 0, -10));
            THEN("Every parent's transform has been applied") {
                REQUIRE(utils::equal(p.x, 0.0f));
                REQUIRE(utils::equal(p.y, 0.0f));
                REQUIRE(utils::equal(p.z, -1.0f));
            }
        }
    }
}

SCENARIO("Converting a normal from object to world space") {
    GIVEN("A sphere inside a scaled group inside a rotated group") {
        Geom g1 = make_group();
        g1.set_transform(rotation_y(std::numbers::pi / 2));
        Geom g2 = make_group();
        g2.set_transform(scale<double>(1, 2, 3));
        add_child(g1, g2);
        Geom s = make_sphere();
        s.set_transform(translation<double>(5, 0, 0));
        add_child(g2, s);
        WHEN("An object-space normal is converted into world space") {
            const auto sqrt3_over_3 = static_cast<float>(std::numbers::sqrt3 / 3);
            const auto n = normal_to_world(s, Vector(sqrt3_over_3, sqrt3_over_3, sqrt3_over_3));
            THEN("Every parent's transform has been applied") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(0.2857, 0.4286, -0.8571)));
            }
        }
    }
}

SCENARIO("Finding the normal on a child object") {
    GIVEN("A sphere inside a scaled group inside a rotated group") {
        Geom g1 = make_group();
        g1.set_transform(rotation_y(std::numbers::pi / 2));
        Geom g2 = make_group();
        g2.set_transform(scale<double>(1, 2, 3));
        add_child(g1, g2);
        Geom s = make_sphere();
        s.set_transform(translation<double>(5, 0, 0));
        add_child(g2, s);
        WHEN("The normal is computed at a world-space point on the sphere") {
            const auto n = normal_at(s, Point(1.7321, 1.1547, -5.5774));
            THEN("The normal accounts for every parent's transform") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(0.2857, 0.4286, -0.8571)));
            }
        }
    }
}
