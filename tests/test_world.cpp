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
        s1.m_material.colour = Colour{0.8f, 1.0f, 0.6f};
        s1.m_material.diffuse = 0.7f;
        s1.m_material.specular = 0.2f;

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
                    return areAlmostEqual(s.m_material.colour, s1.m_material.colour)
                        && s.m_material.diffuse == s1.m_material.diffuse
                        && s.m_material.specular == s1.m_material.specular;
                });
                REQUIRE(it != w.objects.end());
            }

            AND_THEN("w contains s2") {
                const auto it = std::ranges::find_if(w.objects, [&](const Sphere& s) {
                    return s.m_transform == s2.m_transform;
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

SCENARIO("There is no shadow when nothing is collinear with point and light") {
    GIVEN("w ← default_world() and p ← point(0, 10, 0)") {
        const auto w = World::create_default_world();
        constexpr Point p{0, 10, 0};

        THEN("is_shadowed(w, p) is false") {
            REQUIRE_FALSE(w.is_shadowed(p));
        }
    }
}

SCENARIO("The shadow when an object is between the point and the light") {
    GIVEN("w ← default_world() and p ← point(10, -10, 10)") {
        const auto w = World::create_default_world();
        constexpr Point p{10, -10, 10};

        THEN("is_shadowed(w, p) is true") {
            REQUIRE(w.is_shadowed(p));
        }
    }
}

SCENARIO("There is no shadow when an object is behind the light") {
    GIVEN("w ← default_world() and p ← point(-20, 20, -20)") {
        const auto w = World::create_default_world();
        constexpr Point p{-20, 20, -20};

        THEN("is_shadowed(w, p) is false") {
            REQUIRE_FALSE(w.is_shadowed(p));
        }
    }
}

SCENARIO("There is no shadow when an object is behind the point") {
    GIVEN("w ← default_world() and p ← point(-2, 2, -2)") {
        const auto w = World::create_default_world();
        constexpr Point p{-2, 2, -2};

        THEN("is_shadowed(w, p) is false") {
            REQUIRE_FALSE(w.is_shadowed(p));
        }
    }
}

SCENARIO("shade_hit() is given an intersection in shadow") {
    GIVEN("w ← world() with a light and two spheres, s2 translated to z=10") {
        World w;
        w.light = PointLight{Point{0, 0, -10}, Colour{1, 1, 1}};
        Sphere s1 = Sphere::make_sphere();
        w.objects.push_back(s1);
        Sphere s2 = Sphere::make_sphere();
        s2.set_transform(translation<double>(0, 0, 10));
        w.objects.push_back(s2);
        constexpr Ray r{Point{0, 0, 5}, Vector{0, 0, 1}};
        const Intersection i{s2, 4};

        WHEN("comps ← prepare_computations(i, r) and c ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r);
            const auto c = shade_hit(w, comps);

            THEN("c = color(0.1, 0.1, 0.1)") {
                REQUIRE(areAlmostEqual(c, Colour{0.1f, 0.1f, 0.1f}));
            }
        }
    }
}

SCENARIO("The color with an intersection behind the ray") {
    GIVEN("w ← default_world(), outer and inner spheres with ambient=1, r behind outer") {
        auto w = World::create_default_world();
        w.objects[0].m_material.ambient = 1.0f;
        w.objects[1].m_material.ambient = 1.0f;
        constexpr Ray r{Point{0, 0, 0.75f}, Vector{0, 0, -1}};
        const Colour inner_color = w.objects.at(1).m_material.colour;

        WHEN("c ← color_at(w, r)") {
            const auto c = World::colour_at(w, r);

            THEN("c = inner.material.color") {
                REQUIRE(areAlmostEqual(c, inner_color));
            }
        }
    }
}
