//
// Tests for World scenarios from world.feature
//

#include <algorithm>

#include "World.hpp"
#include "Intersect.hpp"
#include "Light.hpp"
#include "MatrixImpl.hpp"

#include "catch2/catch_test_macros.hpp"

using namespace raytracer;

SCENARIO("Creating a world") {
    GIVEN("w ← world()") {
        const auto w = World();

        THEN("w contains no objects") {
            REQUIRE(w.objects.empty());
        }

        AND_THEN("w has no light source") {
            REQUIRE_FALSE(w.light.has_value());
        }
    }
}

SCENARIO("The default world") {
    GIVEN("The default world is created") {
        constexpr PointLight light{Point{-10, 10, -10}, Colour{1, 1, 1}};

        Sphere s1 = Sphere::make_sphere();
        s1.material.colour = Colour{0.8f, 1.0f, 0.6f};
        s1.material.diffuse = 0.7f;
        s1.material.specular = 0.2f;

        Sphere s2 = Sphere::make_sphere();
        s2.set_transform(scale<double>(0.5, 0.5, 0.5));

        WHEN("w ← default_world()") {
            const auto w = World::create_default_world();

            THEN("w.light = light") {
                REQUIRE(w.light.has_value());
                REQUIRE(w.light->position == light.position);
                REQUIRE(areAlmostEqual(w.light->intensity, light.intensity));
            }

            AND_THEN("w contains s1") {
                const auto it = std::ranges::find_if(w.objects, [&](const Sphere& s) {
                    return areAlmostEqual(s.material.colour, s1.material.colour)
                        && s.material.diffuse == s1.material.diffuse
                        && s.material.specular == s1.material.specular;
                });
                REQUIRE(it != w.objects.end());
            }

            AND_THEN("w contains s2") {
                const auto it = std::ranges::find_if(w.objects, [&](const Sphere& s) {
                    return s.transform == s2.transform;
                });
                REQUIRE(it != w.objects.end());
            }
        }
    }
}

SCENARIO("Intersect a world with a ray") {
    GIVEN("w ← default_world() and r ← ray(point(0, 0, -5), vector(0, 0, 1))") {
        const auto w = World::create_default_world();
        constexpr Ray r{Point{0, 0, -5}, Vector{0, 0, 1}};

        WHEN("xs ← intersect_world(w, r)") {
            const auto xs = World::intersect_world(w, r);

            THEN("xs has 4 intersections with correct t values") {
                REQUIRE(xs.size() == 4);
                REQUIRE(xs[0].t == 4.f);
                REQUIRE(xs[1].t == 4.5f);
                REQUIRE(xs[2].t == 5.5f);
                REQUIRE(xs[3].t == 6.f);
            }
        }
    }
}

SCENARIO("Shading an intersection") {
    GIVEN("w ← default_world(), a ray, the first shape, and an intersection") {
        auto w = World::create_default_world();
        constexpr Ray r{Point{0, 0, -5}, Vector{0, 0, 1}};
        const Sphere shape = w.objects[0];
        const Intersection i{shape, 4};

        WHEN("comps ← prepare_computations(i, r) and c ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r);
            const auto c = shade_hit(w, comps);

            THEN("c = color(0.38066, 0.47583, 0.2855)") {
                REQUIRE(areAlmostEqual(c, Colour{0.38066f, 0.47583f, 0.2855f}));
            }
        }
    }
}

SCENARIO("Shading an intersection from the inside") {
    GIVEN("w ← default_world() with inner light, a ray, the second shape, and an intersection") {
        auto w = World::create_default_world();
        w.light = PointLight{Point{0, 0.25f, 0}, Colour{1, 1, 1}};
        constexpr Ray r{Point{0, 0, 0}, Vector{0, 0, 1}};
        const Sphere shape = w.objects.at(1);
        const Intersection i{shape, 0.5f};

        WHEN("comps ← prepare_computations(i, r) and c ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r);
            const auto c = shade_hit(w, comps);

            THEN("c = color(0.90498, 0.90498, 0.90498)") {
                REQUIRE(areAlmostEqual(c, Colour{0.90498f, 0.90498f, 0.90498f}));
            }
        }
    }
}

SCENARIO("The color when a ray misses") {
    GIVEN("w ← default_world() and r ← ray(point(0, 0, -5), vector(0, 1, 0))") {
        const auto w = World::create_default_world();
        constexpr Ray r{Point{0, 0, -5}, Vector{0, 1, 0}};

        WHEN("c ← color_at(w, r)") {
            const auto c = World::colour_at(w, r);

            THEN("c = color(0, 0, 0)") {
                REQUIRE(areAlmostEqual(c, Colour{0, 0, 0}));
            }
        }
    }
}

SCENARIO("The color when a ray hits") {
    GIVEN("w ← default_world() and r ← ray(point(0, 0, -5), vector(0, 0, 1))") {
        const auto w = World::create_default_world();
        constexpr Ray r{Point{0, 0, -5}, Vector{0, 0, 1}};

        WHEN("c ← color_at(w, r)") {
            const auto c = World::colour_at(w, r);

            THEN("c = color(0.38066, 0.47583, 0.2855)") {
                REQUIRE(areAlmostEqual(c, Colour{0.38066f, 0.47583f, 0.2855f}));
            }
        }
    }
}

SCENARIO("The color with an intersection behind the ray") {
    GIVEN("w ← default_world(), outer and inner spheres with ambient=1, r behind outer") {
        auto w = World::create_default_world();
        w.objects[0].material.ambient = 1.0f;
        w.objects[1].material.ambient = 1.0f;
        constexpr Ray r{Point{0, 0, 0.75f}, Vector{0, 0, -1}};
        const Colour inner_color = w.objects.at(1).material.colour;

        WHEN("c ← color_at(w, r)") {
            const auto c = World::colour_at(w, r);

            THEN("c = inner.material.color") {
                REQUIRE(areAlmostEqual(c, inner_color));
            }
        }
    }
}
