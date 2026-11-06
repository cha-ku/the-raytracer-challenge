//
// Created by chaku on 28/12/2025.
//

#include "Intersect.hpp"
#include "Material.hpp"
#include "MatrixImpl.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include <cmath>
#include <numbers>
#include <string>

#include "World.hpp"

using namespace raytracer;

namespace {
    Sphere make_glass_sphere() {
        Sphere s = Sphere::make_sphere();
        s.m_material.transparency = 1.0f;
        s.m_material.refractive_index = 1.5f;
        return s;
    }
}

SCENARIO("Creating and querying a ray") {
    GIVEN("Origin") {
        const Point origin(1, 2, 3);
        const Vector direction(4, 5, 6);
        WHEN("Ray is constructed") {
            Ray ray(origin, direction);

            THEN("Ray origin and direction equals origin and direction") {
                REQUIRE(ray.origin == origin);
                REQUIRE(ray.direction == direction);
            }
        }
    }
}

SCENARIO("Computing a point from a distance") {
    GIVEN("Ray") {
        const Ray ray(Point(2, 3, 4), Vector(1, 0, 0));
        THEN("Position at a distance along the ray") {
            REQUIRE(position(ray, 0) == Point(2, 3, 4));
            REQUIRE(position(ray, 1) == Point(3, 3, 4));
            REQUIRE(position(ray, -1) == Point(1, 3, 4));
            REQUIRE(position(ray, 2.5) == Point(4.5, 3, 4));
        }
    }
}

SCENARIO("A ray intersects a sphere at two points") {
    GIVEN("Ray and Sphere") {
        const Ray ray(Point(0, 0, -5), Vector(0, 0, 1));
        const Sphere sphere = Sphere::make_sphere();
        WHEN("") {
            const auto xs = intersect(sphere, ray);
            THEN("") {
                REQUIRE(xs.size() == 2);
                REQUIRE(xs[0].t == 4.f);
                REQUIRE(xs[1].t == 6.f);
            }
        }
    }
}

SCENARIO("A ray intersects a sphere at a tangent") {
    GIVEN("Ray and Sphere") {
        Ray ray(Point(0, 1, -5), Vector(0, 0, 1));
        Sphere sphere{Sphere::make_sphere()};
        WHEN("") {
            const auto xs = intersect(sphere, ray);
            THEN("") {
                REQUIRE(xs.size() == 2);
                REQUIRE(xs[0].t == 5.f);
                REQUIRE(xs[1].t == 5.f);
            }
        }
    }
}

SCENARIO("A ray misses a sphere") {
    GIVEN("Ray and Sphere") {
        const Ray ray(Point(0, 2, -5), Vector(0, 0, 1));
        const Sphere sphere{Sphere::make_sphere()};
        WHEN("") {
            const auto xs = intersect(sphere, ray);
            THEN("") {
                REQUIRE(xs.empty());
            }
        }
    }
}

SCENARIO("A ray originates inside a sphere") {
    GIVEN("Ray and Sphere") {
        Ray ray(Point(0, 0, 0), Vector(0, 0, 1));
        Sphere sphere{Sphere::make_sphere()};
        WHEN("") {
            const std::vector<Intersection> xs = intersect(sphere, ray);
            THEN("") {
                REQUIRE(xs.size() == 2);
                REQUIRE(xs[0].t == -1.f);
                REQUIRE(xs[1].t == 1.f);
            }
        }
    }
}

SCENARIO("A sphere is behind a ray") {
    GIVEN("Ray and Sphere") {
        Ray ray(Point(0, 0, 5), Vector(0, 0, 1));
        Sphere sphere{Sphere::make_sphere()};
        WHEN("") {
            const std::vector<Intersection> xs = intersect(sphere, ray);
            THEN("") {
                REQUIRE(xs.size() == 2);
                REQUIRE(xs[0].t == -6.f);
                REQUIRE(xs[1].t == -4.f);
            }
        }
    }
}

SCENARIO("An intersection encapsulates t and object") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        WHEN("") {
            const auto [sphere, intersection] = Intersection(s, 3.5);
            THEN("") {
                REQUIRE(intersection == 3.5);
                REQUIRE(sphere == AnyShape{s});
            }
        }
    }
}

SCENARIO("Intersect sets the object on the intersection") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        WHEN("Intersection is created") {
            const Intersection i{s, 3.5};
            THEN("Intersection has correct t and object") {
                REQUIRE(i.t == 3.5f);
                REQUIRE(i.object == AnyShape{s});
            }
        }
    }
}

SCENARIO("Aggregating intersections") {
    GIVEN("Sphere and intersections") {
        const Sphere s = Sphere::make_sphere();
        const Intersection i1{s, 1};
        const Intersection i2{s, 2};
        WHEN("Intersections are aggregated") {
            const auto xs = intersections(i1, i2);
            THEN("Collection contains both intersections in order") {
                REQUIRE(xs.size() == 2);
                REQUIRE(xs[0].t == 1);
                REQUIRE(xs[1].t == 2);
            }
        }
    }
}

SCENARIO("The hit, when all intersections have positive t") {
    GIVEN("Sphere and intersections") {
        const Sphere s = Sphere::make_sphere();
        const Intersection i1{s, 1};
        const Intersection i2{s, 2};
        const auto xs = intersections(i2, i1);
        WHEN("hit is called") {
            const auto i = hit(xs);
            THEN("hit returns the lowest positive intersection") {
                REQUIRE(i.has_value());
                REQUIRE(i->t == i1.t);
                REQUIRE(i->object == i1.object);
            }
        }
    }
}

SCENARIO("The hit, when some intersections have negative t") {
    GIVEN("Sphere and intersections") {
        const Sphere s = Sphere::make_sphere();
        const Intersection i1{s, -1};
        const Intersection i2{s, 1};
        const auto xs = intersections(i2, i1);
        WHEN("hit is called") {
            const auto i = hit(xs);
            THEN("hit returns the lowest positive intersection, ignoring negative") {
                REQUIRE(i.has_value());
                REQUIRE(i->t == i2.t);
                REQUIRE(i->object == i2.object);
            }
        }
    }
}

SCENARIO("The hit, when all intersections have negative t") {
    GIVEN("Sphere and intersections") {
        const Sphere s = Sphere::make_sphere();
        const Intersection i1{s, -2};
        const Intersection i2{s, -1};
        const auto xs = intersections(i2, i1);
        WHEN("hit is called") {
            const auto i = hit(xs);
            THEN("hit returns nothing") {
                REQUIRE_FALSE(i.has_value());
            }
        }
    }
}

SCENARIO("The hit is always the lowest nonnegative intersection") {
    GIVEN("Sphere and multiple intersections") {
        const Sphere s = Sphere::make_sphere();
        const Intersection i1{s, 5};
        const Intersection i2{s, 7};
        const Intersection i3{s, -3};
        const Intersection i4{s, 2};
        const auto xs = intersections(i1, i2, i3, i4);
        WHEN("hit is called") {
            const auto i = hit(xs);
            THEN("hit returns the lowest nonnegative intersection") {
                REQUIRE(i.has_value());
                REQUIRE(i->t == i4.t);
                REQUIRE(i->object == i4.object);
            }
        }
    }
}

SCENARIO("Translating a ray") {
    GIVEN("Ray and translation matrix") {
        const Ray r{Point(1, 2, 3), Vector(0, 1, 0)};
        const auto m = translation<double>(3, 4, 5);
        WHEN("Ray is transformed") {
            const auto r2 = transform(r, m);
            THEN("Origin is translated and direction is unchanged") {
                REQUIRE(r2.origin == Point(4, 6, 8));
                REQUIRE(Vector::areAlmostEqual(r2.direction, Vector(0, 1, 0)));
            }
        }
    }
}

SCENARIO("Scaling a ray") {
    GIVEN("Ray and scaling matrix") {
        const Ray r{Point(1, 2, 3), Vector(0, 1, 0)};
        const auto m = scale<double>(2, 3, 4);
        WHEN("Ray is transformed") {
            const auto r2 = transform(r, m);
            THEN("Both origin and direction are scaled") {
                REQUIRE(r2.origin == Point(2, 6, 12));
                REQUIRE(Vector::areAlmostEqual(r2.direction, Vector(0, 3, 0)));
            }
        }
    }
}

SCENARIO("A sphere's default transformation") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        THEN("Transform is identity matrix") {
            REQUIRE(s.m_transform == Matrix<double>::identity(4));
        }
    }
}

SCENARIO("Changing a sphere's transformation") {
    GIVEN("Sphere and translation") {
        Sphere s = Sphere::make_sphere();
        const auto t = translation<double>(2, 3, 4);
        WHEN("set_transform is called") {
            s.set_transform(t);
            THEN("Transform equals the translation") {
                REQUIRE(s.m_transform == t);
            }
        }
    }
}

SCENARIO("The normal on a sphere at a point on the x axis") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        WHEN("Normal is computed at point on x axis") {
            const auto n = normal_at(s, Point(1, 0, 0));
            THEN("Normal equals vector(1, 0, 0)") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(1, 0, 0)));
            }
        }
    }
}

SCENARIO("The normal on a sphere at a point on the y axis") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        WHEN("Normal is computed at point on y axis") {
            const auto n = normal_at(s, Point(0, 1, 0));
            THEN("Normal equals vector(0, 1, 0)") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(0, 1, 0)));
            }
        }
    }
}

SCENARIO("The normal on a sphere at a point on the z axis") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        WHEN("Normal is computed at point on z axis") {
            const auto n = normal_at(s, Point(0, 0, 1));
            THEN("Normal equals vector(0, 0, 1)") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(0, 0, 1)));
            }
        }
    }
}

SCENARIO("The normal on a sphere at a nonaxial point") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        const auto sqrt3_over_3 = std::sqrt(3) / 3;
        WHEN("Normal is computed at nonaxial point") {
            const auto n = normal_at(s, Point(sqrt3_over_3, sqrt3_over_3, sqrt3_over_3));
            THEN("Normal equals vector(√3/3, √3/3, √3/3)") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(sqrt3_over_3, sqrt3_over_3, sqrt3_over_3)));
            }
        }
    }
}

SCENARIO("The normal is a normalized vector") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        const auto sqrt3_over_3 = std::sqrt(3) / 3;
        WHEN("Normal is computed at nonaxial point") {
            const auto n = normal_at(s, Point(sqrt3_over_3, sqrt3_over_3, sqrt3_over_3));
            THEN("Normal equals normalize(n)") {
                REQUIRE(Vector::areAlmostEqual(n, Vector::normalize(n)));
            }
        }
    }
}

SCENARIO("Computing the normal on a translated sphere") {
    GIVEN("Sphere with translation") {
        Sphere s = Sphere::make_sphere();
        s.set_transform(translation<double>(0, 1, 0));
        WHEN("Normal is computed") {
            const auto n = normal_at(s, Point(0, 1.70711, -0.70711));
            THEN("Normal accounts for translation") {
                REQUIRE(Vector::areAlmostEqual(n, Vector(0, 0.707107, -0.707107)));
            }
        }
    }
}

SCENARIO("Computing the normal on a transformed sphere") {
    GIVEN("Sphere with scaling and rotation") {
        Sphere s = Sphere::make_sphere();
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

SCENARIO("Reflecting a vector approaching at 45 degrees") {
    GIVEN("A vector and a normal") {
        const Vector v(1, -1, 0);
        const Vector n(0, 1, 0);
        WHEN("r = reflect(v, n)") {
            const auto r = Vector::reflect(v, n);
            THEN("r = vector(1, 1, 0)") {
                REQUIRE(Vector::areAlmostEqual(r, Vector(1, 1, 0)));
            }
        }
    }
}

SCENARIO("Reflecting a vector off a slanted surface") {
    GIVEN("A vector and a normal") {
        const auto sqrt2_over_2 = std::sqrt(2.0f) / 2.0f;
        const Vector v(0, -1, 0);
        const Vector n(sqrt2_over_2, sqrt2_over_2, 0);
        WHEN("r = reflect(v, n)") {
            const auto r = Vector::reflect(v, n);
            THEN("r = vector(1, 0, 0)") {
                REQUIRE(Vector::areAlmostEqual(r, Vector(1, 0, 0)));
            }
        }
    }
}

SCENARIO("A sphere has a default material") {
    GIVEN("Sphere") {
        const Sphere s = Sphere::make_sphere();
        WHEN("Material is retrieved") {
            const auto m = s.m_material;
            THEN("Material equals the default material") {
                REQUIRE(m == Material{});
            }
        }
    }
}

SCENARIO("Precomputing the state of an intersection") {
    GIVEN("A ray, a sphere, and an intersection") {
        const Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
        const Sphere shape = Sphere::make_sphere();
        const Intersection i{shape, 4};
        WHEN("comps = prepare_computations(i, r)") {
            const auto comps = prepare_computations(i, r, {i});
            THEN("comps contains precomputed state") {
                REQUIRE(comps.t == i.t);
                REQUIRE(comps.object == i.object);
                REQUIRE(comps.point == Point(0, 0, -1));
                REQUIRE(Vector::areAlmostEqual(comps.eye_vector, Vector(0, 0, -1)));
                REQUIRE(Vector::areAlmostEqual(comps.normal_vector, Vector(0, 0, -1)));
            }
        }
    }
}

SCENARIO("Precomputing the reflection vector") {
    GIVEN("shape ← plane(), r ← ray(point(0, 1, -1), vector(0, -√2/2, √2/2)) and i ← intersection(√2, shape)") {
        constexpr float half_sqrt2 = std::numbers::sqrt2_v<float> / 2;
        const Plane shape = Plane::make_plane();
        const Ray r{
            .origin = Point{.x = 0, .y = 1, .z = -1},
            .direction = Vector{.x = 0, .y = -half_sqrt2, .z = half_sqrt2}
        };
        const Intersection i{shape, std::numbers::sqrt2_v<float>};

        WHEN("comps ← prepare_computations(i, r)") {
            const auto comps = prepare_computations(i, r, {i});

            THEN("comps.reflectv = vector(0, √2/2, √2/2)") {
                REQUIRE(Vector::areAlmostEqual(comps.reflect_vector, Vector{.x = 0, .y = half_sqrt2, .z = half_sqrt2}));
            }
        }
    }
}

SCENARIO("The hit, when an intersection occurs on the outside") {
    GIVEN("A ray, a sphere, and an intersection") {
        const Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
        const Sphere shape = Sphere::make_sphere();
        const Intersection i{shape, 4};
        WHEN("comps = prepare_computations(i, r)") {
            const auto comps = prepare_computations(i, r, {i});
            THEN("comps.inside is false") {
                REQUIRE_FALSE(comps.inside);
            }
        }
    }
}

SCENARIO("The hit, when an intersection occurs on the inside") {
    GIVEN("A ray from origin, a sphere, and an intersection") {
        constexpr Ray r{Point(0, 0, 0), Vector(0, 0, 1)};
        const Sphere shape = Sphere::make_sphere();
        const Intersection i{shape, 1};
        WHEN("comps = prepare_computations(i, r)") {
            const auto comps = prepare_computations(i, r, {i});
            THEN("comps reflects inside hit with inverted normal") {
                REQUIRE(comps.point == Point(0, 0, 1));
                REQUIRE(Vector::areAlmostEqual(comps.eye_vector, Vector(0, 0, -1)));
                REQUIRE(comps.inside);
                REQUIRE(Vector::areAlmostEqual(comps.normal_vector, Vector(0, 0, -1)));
            }
        }
    }
}

SCENARIO("A helper for producing a sphere with a glassy material") {
    GIVEN("s ← glass_sphere()") {
        const Sphere s = make_glass_sphere();

        THEN("s.transform = identity_matrix") {
            REQUIRE(s.m_transform == Matrix<double>::identity(4));
        }
        AND_THEN("s.material.transparency = 1.0") {
            REQUIRE(s.m_material.transparency == 1.0f);
        }
        AND_THEN("s.material.refractive_index = 1.5") {
            REQUIRE(s.m_material.refractive_index == 1.5f);
        }
    }
}

SCENARIO("A sphere may be assigned a material") {
    GIVEN("Sphere and material") {
        Sphere s = Sphere::make_sphere();
        Material m{};
        m.ambient = 1;
        WHEN("Material is assigned to sphere") {
            s.m_material = m;
            THEN("Sphere has the assigned material") {
                REQUIRE(s.m_material == m);
            }
        }
    }
}

SCENARIO("The hit should offset the point") {
    GIVEN("r ← ray(point(0, 0, -5), vector(0, 0, 1)) and shape ← sphere with translation(0, 0, 1)") {
        constexpr Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
        Sphere shape = Sphere::make_sphere();
        shape.set_transform(translation<double>(0, 0, 1));
        const Intersection i{shape, 5};

        WHEN("comps ← prepare_computations(i, r)") {
            const auto comps = prepare_computations(i, r, {i});

            THEN("comps.over_point.z < -EPSILON/2 and comps.point.z > comps.over_point.z") {
                REQUIRE(comps.over_point.z < -utils::EPSILON / 2);
                REQUIRE(comps.point.z > comps.over_point.z);
            }
        }
    }
}

SCENARIO("The under point is offset below the surface") {
    GIVEN("r ← ray(point(0, 0, -5), vector(0, 0, 1)), shape ← glass_sphere() with translation(0, 0, 1), "
          "i ← intersection(5, shape) and xs ← intersections(i)") {
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = -5}, .direction = Vector{.x = 0, .y = 0, .z = 1}};
        Sphere shape = make_glass_sphere();
        shape.set_transform(translation<double>(0, 0, 1));
        const Intersection i{.object = shape, .t = 5};
        const auto xs = intersections(i);

        WHEN("comps ← prepare_computations(i, r, xs)") {
            const auto comps = prepare_computations(i, r, xs);

            THEN("comps.under_point.z > EPSILON/2") {
                REQUIRE(comps.under_point.z > utils::EPSILON / 2);
            }
            AND_THEN("comps.point.z < comps.under_point.z") {
                REQUIRE(comps.point.z < comps.under_point.z);
            }
        }
    }
}

SCENARIO("Finding n1 and n2 at various intersections") {
    GIVEN("A ← glass_sphere() with scaling(2, 2, 2) and refractive_index 1.5, "
          "B ← glass_sphere() with translation(0, 0, -0.25) and refractive_index 2.0, "
          "C ← glass_sphere() with translation(0, 0, 0.25) and refractive_index 2.5, "
          "r ← ray(point(0, 0, -4), vector(0, 0, 1)) and "
          "xs ← intersections(2:A, 2.75:B, 3.25:C, 4.75:B, 5.25:C, 6:A)") {
        Sphere a = make_glass_sphere();
        a.set_transform(scale<double>(2, 2, 2));
        a.m_material.refractive_index = 1.5f;
        Sphere b = make_glass_sphere();
        b.set_transform(translation<double>(0, 0, -0.25));
        b.m_material.refractive_index = 2.0f;
        Sphere c = make_glass_sphere();
        c.set_transform(translation<double>(0, 0, 0.25));
        c.m_material.refractive_index = 2.5f;
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = -4}, .direction = Vector{.x = 0, .y = 0, .z = 1}};
        const auto xs = intersections(Intersection{.object = a, .t = 2},
                                      Intersection{.object = b, .t = 2.75f},
                                      Intersection{.object = c, .t = 3.25f},
                                      Intersection{.object = b, .t = 4.75f},
                                      Intersection{.object = c, .t = 5.25f},
                                      Intersection{.object = a, .t = 6});

        // Scenario Outline examples: | index | n1 | n2 |
        const auto [index, n1, n2] = GENERATE(table<std::size_t, float, float>({
            {0, 1.0f, 1.5f},
            {1, 1.5f, 2.0f},
            {2, 2.0f, 2.5f},
            {3, 2.5f, 2.5f},
            {4, 2.5f, 1.5f},
            {5, 1.5f, 1.0f},
        }));

        WHEN("comps ← prepare_computations(xs[" + std::to_string(index) + "], r, xs)") {
            const auto comps = prepare_computations(xs[index], r, xs);

            THEN("comps.n1 = " + std::to_string(n1)) {
                REQUIRE(comps.n1 == n1);
            }
            AND_THEN("comps.n2 = " + std::to_string(n2)) {
                REQUIRE(comps.n2 == n2);
            }
        }
    }
}

SCENARIO("The Schlick approximation under total internal reflection") {
    GIVEN("shape ← glass_sphere(), r ← ray(point(0, 0, √2/2), vector(0, 1, 0)) "
          "and xs ← intersections(-√2/2:shape, √2/2:shape)") {
        constexpr float half_sqrt2 = std::numbers::sqrt2_v<float> / 2;
        const Sphere shape = make_glass_sphere();
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = half_sqrt2}, .direction = Vector{.x = 0, .y = 1, .z = 0}};
        const auto xs = intersections(Intersection{.object = shape, .t = -half_sqrt2},
                                      Intersection{.object = shape, .t = half_sqrt2});

        WHEN("comps ← prepare_computations(xs[1], r, xs) and reflectance ← schlick(comps)") {
            const auto comps = prepare_computations(xs[1], r, xs);
            const auto reflectance = World::schlick(comps);

            THEN("reflectance = 1.0") {
                REQUIRE(utils::equal(reflectance, 1.0f));
            }
        }
    }
}

SCENARIO("The Schlick approximation with a perpendicular viewing angle") {
    GIVEN("shape ← glass_sphere(), r ← ray(point(0, 0, 0), vector(0, 1, 0)) "
          "and xs ← intersections(-1:shape, 1:shape)") {
        const Sphere shape = make_glass_sphere();
        constexpr Ray r{.origin = Point{.x = 0, .y = 0, .z = 0}, .direction = Vector{.x = 0, .y = 1, .z = 0}};
        const auto xs = intersections(Intersection{.object = shape, .t = -1}, Intersection{.object = shape, .t = 1});

        WHEN("comps ← prepare_computations(xs[1], r, xs) and reflectance ← schlick(comps)") {
            const auto comps = prepare_computations(xs[1], r, xs);
            const auto reflectance = World::schlick(comps);

            THEN("reflectance = 0.04") {
                REQUIRE(utils::equal(reflectance, 0.04f));
            }
        }
    }
}

SCENARIO("The Schlick approximation with small angle and n2 > n1") {
    GIVEN("shape ← glass_sphere(), r ← ray(point(0, 0.99, -2), vector(0, 0, 1)) "
          "and xs ← intersections(1.8589:shape)") {
        const Sphere shape = make_glass_sphere();
        constexpr Ray r{.origin = Point{.x = 0, .y = 0.99f, .z = -2}, .direction = Vector{.x = 0, .y = 0, .z = 1}};
        const auto xs = intersections(Intersection{.object = shape, .t = 1.8589f});

        WHEN("comps ← prepare_computations(xs[0], r, xs) and reflectance ← schlick(comps)") {
            const auto comps = prepare_computations(xs[0], r, xs);
            const auto reflectance = World::schlick(comps);

            THEN("reflectance = 0.48873") {
                REQUIRE(utils::equal(reflectance, 0.48873f));
            }
        }
    }
}
