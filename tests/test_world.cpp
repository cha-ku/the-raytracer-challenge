//
// Tests for World scenarios from world.feature
//

#include <algorithm>
#include <numbers>

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
                const auto it = std::ranges::find_if(w.objects, [&](const AnyShape& s) {
                    return areAlmostEqual(material_of(s).colour, s1.m_material.colour)
                        && material_of(s).diffuse == s1.m_material.diffuse
                        && material_of(s).specular == s1.m_material.specular;
                });
                REQUIRE(it != w.objects.end());
            }

            AND_THEN("w contains s2") {
                const auto it = std::ranges::find_if(w.objects, [&](const AnyShape& s) {
                    return transform_of(s) == s2.m_transform;
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
        const Sphere shape = std::get<Sphere>(w.objects[0]);
        const Intersection i{shape, 4};

        WHEN("comps ← prepare_computations(i, r) and c ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r);
            const auto c = shade_hit(w, comps, 255);

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
        const Sphere shape = std::get<Sphere>(w.objects.at(1));
        const Intersection i{shape, 0.5f};

        WHEN("comps ← prepare_computations(i, r) and c ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r);
            const auto c = shade_hit(w, comps, 255);

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
            const auto c = World::colour_at(w, r, 255);

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
            const auto c = World::colour_at(w, r, 255);

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
            const auto c = shade_hit(w, comps, 255);

            THEN("c = color(0.1, 0.1, 0.1)") {
                REQUIRE(areAlmostEqual(c, Colour{0.1f, 0.1f, 0.1f}));
            }
        }
    }
}

SCENARIO("The color with an intersection behind the ray") {
    GIVEN("w ← default_world(), outer and inner spheres with ambient=1, r behind outer") {
        auto w = World::create_default_world();
        material_of(w.objects[0]).ambient = 1.0f;
        material_of(w.objects[1]).ambient = 1.0f;
        constexpr Ray r{Point{0, 0, 0.75f}, Vector{0, 0, -1}};
        const Colour inner_color = material_of(w.objects.at(1)).colour;

        WHEN("c ← color_at(w, r)") {
            const auto c = World::colour_at(w, r, 255);

            THEN("c = inner.material.color") {
                REQUIRE(areAlmostEqual(c, inner_color));
            }
        }
    }
}

SCENARIO("The reflected color for a nonreflective material") {
    GIVEN("w ← default_world(), r ← ray(point(0, 0, 0), vector(0, 0, 1)), shape ← the second object in w, "
          "shape.material.ambient ← 1 and i ← intersection(1, shape)") {
        auto w = World::create_default_world();
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = 0}, .direction = Vector{.x = 0, .y = 0, .z = 1}};
        AnyShape &shape = w.objects[1];
        material_of(shape).ambient = 1.0f;
        const Intersection i{.object = shape, .t = 1};

        WHEN("comps ← prepare_computations(i, r) and color ← reflected_color(w, comps)") {
            const auto comps = prepare_computations(i, r);
            const auto colour = World::reflected_colour(w, comps, 255);

            THEN("color = color(0, 0, 0)") {
                REQUIRE(areAlmostEqual(colour, Colour{.r = 0, .g = 0, .b = 0}));
            }
        }
    }
}

SCENARIO("The reflected color for a reflective material") {
    GIVEN("w ← default_world(), shape ← plane() with material.reflective 0.5 and transform translation(0, -1, 0), "
          "shape is added to w, r ← ray(point(0, 0, -3), vector(0, -√2/2, √2/2)) and i ← intersection(√2, shape)") {
        constexpr float half_sqrt2 = std::numbers::sqrt2_v<float> / 2;
        auto w = World::create_default_world();
        Plane shape = Plane::make_plane();
        shape.m_material.reflective = 0.5f;
        shape.set_transform(translation<double>(0, -1, 0));
        w.objects.push_back(shape);
        constexpr Ray r{
            .origin = Point{.x = 0, .y = 0, .z = -3},
            .direction = Vector{.x = 0, .y = -half_sqrt2, .z = half_sqrt2}
        };
        const Intersection i{.object = shape, .t = std::numbers::sqrt2_v<float>};

        WHEN("comps ← prepare_computations(i, r) and color ← reflected_color(w, comps)") {
            const auto comps = prepare_computations(i, r);
            const auto colour = World::reflected_colour(w, comps, 255);

            THEN("color = color(0.19032, 0.2379, 0.14274)") {
                REQUIRE(areAlmostEqual(colour, Colour{.r = 0.19032f, .g = 0.2379f, .b = 0.14274f}));
            }
        }
    }
}

SCENARIO("shade_hit() with a reflective material") {
    GIVEN("w ← default_world(), shape ← plane() with material.reflective 0.5 and transform translation(0, -1, 0), "
          "shape is added to w, r ← ray(point(0, 0, -3), vector(0, -√2/2, √2/2)) and i ← intersection(√2, shape)") {
        constexpr float half_sqrt2 = std::numbers::sqrt2_v<float> / 2;
        auto w = World::create_default_world();
        Plane shape = Plane::make_plane();
        shape.m_material.reflective = 0.5f;
        shape.set_transform(translation<double>(0, -1, 0));
        w.objects.push_back(shape);
        constexpr Ray r{
            .origin = Point{.x = 0, .y = 0, .z = -3},
            .direction = Vector{.x = 0, .y = -half_sqrt2, .z = half_sqrt2}
        };
        const Intersection i{.object = shape, .t = std::numbers::sqrt2_v<float>};

        WHEN("comps ← prepare_computations(i, r) and color ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r);
            const auto colour = shade_hit(w, comps, 255);

            THEN("color = color(0.87677, 0.92436, 0.82918)") {
                REQUIRE(areAlmostEqual(colour, Colour{.r = 0.87677f, .g = 0.92436f, .b = 0.82918f}));
            }
        }
    }
}

SCENARIO("color_at() with mutually reflective surfaces") {
    GIVEN("w ← world() with w.light ← point_light(point(0, 0, 0), color(1, 1, 1)), "
          "lower ← plane() with material.reflective 1 and transform translation(0, -1, 0), "
          "upper ← plane() with material.reflective 1 and transform translation(0, 1, 0), "
          "both added to w, and r ← ray(point(0, 0, 0), vector(0, 1, 0))") {
        World w;
        w.light = PointLight{.position = Point{.x = 0, .y = 0, .z = 0}, .intensity = Colour{.r = 1, .g = 1, .b = 1}};
        Plane lower = Plane::make_plane();
        lower.m_material.reflective = 1.0f;
        lower.set_transform(translation<double>(0, -1, 0));
        w.objects.emplace_back(lower);
        Plane upper = Plane::make_plane();
        upper.m_material.reflective = 1.0f;
        upper.set_transform(translation<double>(0, 1, 0));
        w.objects.emplace_back(upper);
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = 0}, .direction = Vector{.x = 0, .y = 1, .z = 0}};

        THEN("color_at(w, r) should terminate successfully") {
            // Without a recursion limit the ray bounces between the two planes forever and this
            // overflows the stack (crashing the test binary) rather than throwing; returning at all is the pass
            REQUIRE_NOTHROW(World::colour_at(w, r, 255));
        }
    }
}
