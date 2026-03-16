//
// Created by chaku on 28/12/2025.
//

#include "Intersect.hpp"
#include "Material.hpp"
#include "MatrixImpl.hpp"

#include "catch2/catch_test_macros.hpp"
#include <cmath>
#include <numbers>

using namespace raytracer;

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
                REQUIRE(sphere == s);
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
                REQUIRE(i.object == s);
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
            REQUIRE(s.transform == Container<double>::identity(4));
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
                REQUIRE(s.transform == t);
            }
        }
    }
}

SCENARIO("Intersecting a scaled sphere with a ray") {
    GIVEN("Ray and sphere") {
        const Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
        Sphere s = Sphere::make_sphere();
        WHEN("Sphere is scaled and intersected") {
            s.set_transform(scale<double>(2, 2, 2));
            const auto xs = intersect(s, r);
            THEN("Intersections are at 3 and 7") {
                REQUIRE(xs.size() == 2);
                REQUIRE(xs[0].t == 3);
                REQUIRE(xs[1].t == 7);
            }
        }
    }
}

SCENARIO("Intersecting a translated sphere with a ray") {
    GIVEN("Ray and sphere") {
        const Ray r{Point(0, 0, -5), Vector(0, 0, 1)};
        Sphere s = Sphere::make_sphere();
        WHEN("Sphere is translated and intersected") {
            s.set_transform(translation<double>(5, 0, 0));
            const auto xs = intersect(s, r);
            THEN("Ray misses the sphere") {
                REQUIRE(xs.empty());
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
            const auto m = s.material;
            THEN("Material equals the default material") {
                REQUIRE(m == Material{});
            }
        }
    }
}

SCENARIO("A sphere may be assigned a material") {
    GIVEN("Sphere and material") {
        Sphere s = Sphere::make_sphere();
        Material m{};
        m.ambient = 1;
        WHEN("Material is assigned to sphere") {
            s.material = m;
            THEN("Sphere has the assigned material") {
                REQUIRE(s.material == m);
            }
        }
    }
}
