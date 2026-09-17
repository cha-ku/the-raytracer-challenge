#include "MatrixImpl.hpp"
#include "TestShape.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

SCENARIO("The default transformation") {
    GIVEN("Shape") {
        const auto s = test_shape();
        THEN("Transform is identity matrix") {
            REQUIRE(s.m_transform == Matrix<double>::identity(4));
        }
    }
}

SCENARIO("Assigning a transformation") {
    GIVEN("Shape and translation") {
        auto s = test_shape();
        const auto t = translation<double>(2, 3, 4);
        WHEN("set_transform is called") {
            s.set_transform(t);
            THEN("Transform equals the translation") {
                REQUIRE(s.m_transform == t);
            }
        }
    }
}

SCENARIO("A shape's default material") {
    GIVEN("Shape") {
        const auto s = test_shape();
        WHEN("Material is retrieved") {
            const auto m = s.m_material;
            THEN("Material equals the default material") {
                REQUIRE(m == Material{});
            }
        }
    }
}

SCENARIO("Assigning a material to a shape") {
    GIVEN("Shape and material") {
        auto s = test_shape();
        Material m{};
        m.ambient = 1;
        WHEN("Material is assigned to shape") {
            s.m_material = m;
            THEN("Shape has the assigned material") {
                REQUIRE(s.m_material == m);
            }
        }
    }
}
