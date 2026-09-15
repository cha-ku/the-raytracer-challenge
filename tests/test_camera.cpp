//
// Tests for Camera scenarios from camera.feature
//

#include <numbers>
#include <cmath>

#include "Camera.hpp"
#include "Intersect.hpp"
#include "MatrixImpl.hpp"
#include "World.hpp"
#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

SCENARIO("Constructing a camera") {
    GIVEN("hsize ← 160, vsize ← 120, field_of_view ← π/2") {
        constexpr uint32_t hsize{160};
        constexpr uint32_t vsize{120};
        constexpr double field_of_view{std::numbers::pi / 2};

        WHEN("c ← camera(hsize, vsize, field_of_view)") {
            const Camera c{hsize, vsize, field_of_view};

            THEN("c.hsize = 160 and c.vsize = 120 and c.field_of_view = π/2 and c.transform = identity_matrix") {
                REQUIRE(c.hsize == 160);
                REQUIRE(c.vsize == 120);
                REQUIRE(c.field_of_view == std::numbers::pi / 2);
                REQUIRE(c.transform == Matrix<double>::identity(4));
            }
        }
    }
}

SCENARIO("The pixel size for a horizontal canvas") {
    GIVEN("c ← camera(200, 125, π/2)") {
        const Camera c{200, 125, std::numbers::pi / 2};

        THEN("c.pixel_size = 0.01") {
            REQUIRE(std::abs(c.pixel_size - 0.01) < 1e-5);
        }
    }
}

SCENARIO("The pixel size for a vertical canvas") {
    GIVEN("c ← camera(125, 200, π/2)") {
        const Camera c{125, 200, std::numbers::pi / 2};

        THEN("c.pixel_size = 0.01") {
            REQUIRE(std::abs(c.pixel_size - 0.01) < 1e-5);
        }
    }
}

SCENARIO("Constructing a ray through the center of the canvas") {
    GIVEN("c ← camera(201, 101, π/2)") {
        const Camera c{201, 101, std::numbers::pi / 2};

        WHEN("r ← ray_for_pixel(c, 100, 50)") {
            const Ray r = World::ray_for_pixel(c, 100, 50);

            THEN("r.origin = point(0, 0, 0) and r.direction = vector(0, 0, -1)") {
                REQUIRE(r.origin == Point{0, 0, 0});
                REQUIRE(Vector::areAlmostEqual(r.direction, Vector{0, 0, -1}));
            }
        }
    }
}

SCENARIO("Constructing a ray through a corner of the canvas") {
    GIVEN("c ← camera(201, 101, π/2)") {
        const Camera c{201, 101, std::numbers::pi / 2};

        WHEN("r ← ray_for_pixel(c, 0, 0)") {
            const Ray r = World::ray_for_pixel(c, 0, 0);

            THEN("r.origin = point(0, 0, 0) and r.direction = vector(0.66519, 0.33259, -0.66851)") {
                REQUIRE(r.origin == Point{0, 0, 0});
                REQUIRE(Vector::areAlmostEqual(r.direction, Vector{0.66519f, 0.33259f, -0.66851f}));
            }
        }
    }
}

SCENARIO("Constructing a ray when the camera is transformed") {
    GIVEN("c ← camera(201, 101, π/2)") {
        Camera c{201, 101, std::numbers::pi / 2};

        WHEN("c.transform ← rotation_y(π/4) * translation(0, -2, 5) and r ← ray_for_pixel(c, 100, 50)") {
            c.transform = multiply(rotation_y(std::numbers::pi / 4), translation<double>(0, -2, 5));
            const Ray r = World::ray_for_pixel(c, 100, 50);

            THEN("r.origin = point(0, 2, -5) and r.direction = vector(√2/2, 0, -√2/2)") {
                REQUIRE(r.origin == Point{0, 2, -5});
                const auto sq2_2 = static_cast<float>(std::numbers::sqrt2 / 2);
                REQUIRE(Vector::areAlmostEqual(r.direction, Vector{sq2_2, 0, -sq2_2}));
            }
        }
    }
}
