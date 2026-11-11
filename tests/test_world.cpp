//
// Tests for World scenarios from world.feature
//

#include <algorithm>
#include <numbers>

#include "World.hpp"
#include "Intersect.hpp"
#include "Light.hpp"
#include "MatrixImpl.hpp"
#include "TestPattern.hpp"

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

        Geom s1 = make_sphere();
        s1.material.colour = Colour{0.8f, 1.0f, 0.6f};
        s1.material.diffuse = 0.7f;
        s1.material.specular = 0.2f;

        Geom s2 = make_sphere();
        s2.set_transform(scale<double>(0.5, 0.5, 0.5));

        WHEN("w ← default_world()") {
            const auto w = World::create_default_world();

            THEN("w.light = light") {
                REQUIRE(w.light.has_value());
                REQUIRE(w.light->position == light.position);
                REQUIRE(areAlmostEqual(w.light->intensity, light.intensity));
            }

            AND_THEN("w contains s1") {
                const auto it = std::ranges::find_if(w.objects, [&](const std::unique_ptr<Geom> &s) {
                    return areAlmostEqual(s->material.colour, s1.material.colour)
                        && s->material.diffuse == s1.material.diffuse
                        && s->material.specular == s1.material.specular;
                });
                REQUIRE(it != w.objects.end());
            }

            AND_THEN("w contains s2") {
                const auto it = std::ranges::find_if(w.objects, [&](const std::unique_ptr<Geom> &s) {
                    return s->transform() == s2.transform();
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
        const Geom &shape = *w.objects[0];
        const Intersection i{&shape, 4};

        WHEN("comps ← prepare_computations(i, r) and c ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r, {i});
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
        const Geom &shape = *w.objects.at(1);
        const Intersection i{&shape, 0.5f};

        WHEN("comps ← prepare_computations(i, r) and c ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r, {i});
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
        w.add(Sphere{});
        Geom &s2 = w.add(Sphere{});
        s2.set_transform(translation<double>(0, 0, 10));
        constexpr Ray r{Point{0, 0, 5}, Vector{0, 0, 1}};
        const Intersection i{&s2, 4};

        WHEN("comps ← prepare_computations(i, r) and c ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r, {i});
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
        w.objects[0]->material.ambient = 1.0f;
        w.objects[1]->material.ambient = 1.0f;
        constexpr Ray r{Point{0, 0, 0.75f}, Vector{0, 0, -1}};
        const Colour inner_color = w.objects.at(1)->material.colour;

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
        Geom &shape = *w.objects[1];
        shape.material.ambient = 1.0f;
        const Intersection i{.object = &shape, .t = 1};

        WHEN("comps ← prepare_computations(i, r) and color ← reflected_color(w, comps)") {
            const auto comps = prepare_computations(i, r, {i});
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
        Geom &shape = w.add(Plane{});
        shape.material.reflective = 0.5f;
        shape.set_transform(translation<double>(0, -1, 0));
        constexpr Ray r{
            .origin = Point{.x = 0, .y = 0, .z = -3},
            .direction = Vector{.x = 0, .y = -half_sqrt2, .z = half_sqrt2}
        };
        const Intersection i{.object = &shape, .t = std::numbers::sqrt2_v<float>};

        WHEN("comps ← prepare_computations(i, r) and color ← reflected_color(w, comps)") {
            const auto comps = prepare_computations(i, r, {i});
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
        Geom &shape = w.add(Plane{});
        shape.material.reflective = 0.5f;
        shape.set_transform(translation<double>(0, -1, 0));
        constexpr Ray r{
            .origin = Point{.x = 0, .y = 0, .z = -3},
            .direction = Vector{.x = 0, .y = -half_sqrt2, .z = half_sqrt2}
        };
        const Intersection i{.object = &shape, .t = std::numbers::sqrt2_v<float>};

        WHEN("comps ← prepare_computations(i, r) and color ← shade_hit(w, comps)") {
            const auto comps = prepare_computations(i, r, {i});
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
        Geom &lower = w.add(Plane{});
        lower.material.reflective = 1.0f;
        lower.set_transform(translation<double>(0, -1, 0));
        Geom &upper = w.add(Plane{});
        upper.material.reflective = 1.0f;
        upper.set_transform(translation<double>(0, 1, 0));
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = 0}, .direction = Vector{.x = 0, .y = 1, .z = 0}};

        THEN("color_at(w, r) should terminate successfully") {
            // Without a recursion limit the ray bounces between the two planes forever and this
            // overflows the stack (crashing the test binary) rather than throwing; returning at all is the pass
            REQUIRE_NOTHROW(World::colour_at(w, r, 255));
        }
    }
}

SCENARIO("The refracted color with an opaque surface") {
    GIVEN("w ← default_world(), shape ← the first object in w, r ← ray(point(0, 0, -5), vector(0, 0, 1)) "
          "and xs ← intersections(4:shape, 6:shape)") {
        const auto w = World::create_default_world();
        const Geom &shape = *w.objects[0];
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = -5}, .direction = Vector{.x = 0, .y = 0, .z = 1}};
        const auto xs = intersections(Intersection{.object = &shape, .t = 4}, Intersection{.object = &shape, .t = 6});

        WHEN("comps ← prepare_computations(xs[0], r, xs) and c ← refracted_color(w, comps, 5)") {
            const auto comps = prepare_computations(xs[0], r, xs);
            const auto c = World::refracted_colour(w, comps, 5);

            THEN("c = color(0, 0, 0)") {
                REQUIRE(areAlmostEqual(c, colours::black));
            }
        }
    }
}

SCENARIO("The refracted color at the maximum recursive depth") {
    GIVEN("w ← default_world(), shape ← the first object in w with transparency 1.0 and refractive_index 1.5, "
          "r ← ray(point(0, 0, -5), vector(0, 0, 1)) and xs ← intersections(4:shape, 6:shape)") {
        auto w = World::create_default_world();
        Geom &shape = *w.objects[0];
        shape.material.transparency = 1.0f;
        shape.material.refractive_index = 1.5f;
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = -5}, .direction = Vector{.x = 0, .y = 0, .z = 1}};
        const auto xs = intersections(Intersection{.object = &shape, .t = 4}, Intersection{.object = &shape, .t = 6});

        WHEN("comps ← prepare_computations(xs[0], r, xs) and c ← refracted_color(w, comps, 0)") {
            const auto comps = prepare_computations(xs[0], r, xs);
            const auto c = World::refracted_colour(w, comps, 0);

            THEN("c = color(0, 0, 0)") {
                REQUIRE(areAlmostEqual(c, colours::black));
            }
        }
    }
}

SCENARIO("The refracted color under total internal reflection") {
    GIVEN("w ← default_world(), shape ← the first object in w with transparency 1.0 and refractive_index 1.5, "
          "r ← ray(point(0, 0, √2/2), vector(0, 1, 0)) and xs ← intersections(-√2/2:shape, √2/2:shape)") {
        constexpr float half_sqrt2 = std::numbers::sqrt2_v<float> / 2;
        auto w = World::create_default_world();
        Geom &shape = *w.objects[0];
        shape.material.transparency = 1.0f;
        shape.material.refractive_index = 1.5f;
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = half_sqrt2}, .direction = Vector{.x = 0, .y = 1, .z = 0}};
        const auto xs = intersections(Intersection{.object = &shape, .t = -half_sqrt2},
                                      Intersection{.object = &shape, .t = half_sqrt2});

        // The ray starts inside the sphere, so the hit to look at is the second intersection, xs[1]
        WHEN("comps ← prepare_computations(xs[1], r, xs) and c ← refracted_color(w, comps, 5)") {
            const auto comps = prepare_computations(xs[1], r, xs);
            const auto c = World::refracted_colour(w, comps, 5);

            THEN("c = color(0, 0, 0)") {
                REQUIRE(areAlmostEqual(c, colours::black));
            }
        }
    }
}

SCENARIO("The refracted color with a refracted ray") {
    GIVEN("w ← default_world(), A ← the first object in w with ambient 1.0 and pattern test_pattern(), "
          "B ← the second object in w with transparency 1.0 and refractive_index 1.5, "
          "r ← ray(point(0, 0, 0.1), vector(0, 1, 0)) and "
          "xs ← intersections(-0.9899:A, -0.4899:B, 0.4899:B, 0.9899:A)") {
        auto w = World::create_default_world();
        Geom &a = *w.objects[0];
        a.material.ambient = 1.0f;
        a.material.pattern = test_pattern();
        Geom &b = *w.objects[1];
        b.material.transparency = 1.0f;
        b.material.refractive_index = 1.5f;
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = 0.1f}, .direction = Vector{.x = 0, .y = 1, .z = 0}};
        const auto xs = intersections(Intersection{.object = &a, .t = -0.9899f},
                                      Intersection{.object = &b, .t = -0.4899f},
                                      Intersection{.object = &b, .t = 0.4899f},
                                      Intersection{.object = &a, .t = 0.9899f});

        WHEN("comps ← prepare_computations(xs[2], r, xs) and c ← refracted_color(w, comps, 5)") {
            const auto comps = prepare_computations(xs[2], r, xs);
            const auto c = World::refracted_colour(w, comps, 5);

            THEN("c = color(0, 0.99888, 0.04725)") {
                REQUIRE(areAlmostEqual(c, Colour{.r = 0, .g = 0.99888f, .b = 0.04725f}));
            }
        }
    }
}

SCENARIO("shade_hit() with a transparent material") {
    GIVEN("w ← default_world(), floor ← plane() with translation(0, -1, 0), transparency 0.5 and refractive_index 1.5, "
          "ball ← sphere() with color (1, 0, 0), ambient 0.5 and translation(0, -3.5, -0.5), both added to w, "
          "r ← ray(point(0, 0, -3), vector(0, -√2/2, √2/2)) and xs ← intersections(√2:floor)") {
        constexpr float half_sqrt2 = std::numbers::sqrt2_v<float> / 2;
        auto w = World::create_default_world();
        Geom &floor = w.add(Plane{});
        floor.set_transform(translation<double>(0, -1, 0));
        floor.material.transparency = 0.5f;
        floor.material.refractive_index = 1.5f;
        Geom &ball = w.add(Sphere{});
        ball.material.colour = colours::red;
        ball.material.ambient = 0.5f;
        ball.set_transform(translation<double>(0, -3.5, -0.5));
        constexpr Ray r{
            .origin = Point{.x = 0, .y = 0, .z = -3},
            .direction = Vector{.x = 0, .y = -half_sqrt2, .z = half_sqrt2}
        };
        const auto xs = intersections(Intersection{.object = &floor, .t = std::numbers::sqrt2_v<float>});

        WHEN("comps ← prepare_computations(xs[0], r, xs) and color ← shade_hit(w, comps, 5)") {
            const auto comps = prepare_computations(xs[0], r, xs);
            const auto colour = shade_hit(w, comps, 5);

            THEN("color = color(0.93642, 0.68642, 0.68642)") {
                REQUIRE(areAlmostEqual(colour, Colour{.r = 0.93642f, .g = 0.68642f, .b = 0.68642f}));
            }
        }
    }
}

SCENARIO("shade_hit() with a reflective, transparent material") {
    GIVEN("w ← default_world(), r ← ray(point(0, 0, -3), vector(0, -√2/2, √2/2)), "
          "floor ← plane() with translation(0, -1, 0), reflective 0.5, transparency 0.5 and refractive_index 1.5, "
          "ball ← sphere() with color (1, 0, 0), ambient 0.5 and translation(0, -3.5, -0.5), both added to w, "
          "and xs ← intersections(√2:floor)") {
        constexpr float half_sqrt2 = std::numbers::sqrt2_v<float> / 2;
        auto w = World::create_default_world();
        constexpr Ray r{
            .origin = Point{.x = 0, .y = 0, .z = -3},
            .direction = Vector{.x = 0, .y = -half_sqrt2, .z = half_sqrt2}
        };
        Geom &floor = w.add(Plane{});
        floor.set_transform(translation<double>(0, -1, 0));
        floor.material.reflective = 0.5f;
        floor.material.transparency = 0.5f;
        floor.material.refractive_index = 1.5f;
        Geom &ball = w.add(Sphere{});
        ball.material.colour = colours::red;
        ball.material.ambient = 0.5f;
        ball.set_transform(translation<double>(0, -3.5, -0.5));
        const auto xs = intersections(Intersection{.object = &floor, .t = std::numbers::sqrt2_v<float>});

        WHEN("comps ← prepare_computations(xs[0], r, xs) and color ← shade_hit(w, comps, 5)") {
            const auto comps = prepare_computations(xs[0], r, xs);
            const auto colour = shade_hit(w, comps, 5);

            THEN("color = color(0.93391, 0.69643, 0.69243)") {
                REQUIRE(areAlmostEqual(colour, Colour{.r = 0.93391f, .g = 0.69643f, .b = 0.69243f}));
            }
        }
    }
}
