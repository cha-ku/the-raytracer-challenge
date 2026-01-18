//
// Created by chaku on 28/12/2025.
//

#include "Intersect.hpp"
#include "MatrixImpl.hpp"

#include "catch2/catch_test_macros.hpp"

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
                REQUIRE(r2.direction == Vector(0, 1, 0));
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
                REQUIRE(r2.direction == Vector(0, 3, 0));
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
            set_transform(s, t);
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
            set_transform(s, scale<double>(2, 2, 2));
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
            set_transform(s, translation<double>(5, 0, 0));
            const auto xs = intersect(s, r);
            THEN("Ray misses the sphere") {
                REQUIRE(xs.empty());
            }
        }
    }
}
