#include "ObjFile.hpp"
#include "Utils.hpp"

#include "catch2/catch_test_macros.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <variant>

using namespace raytracer;

namespace {
    bool points_equal(const Point &a, const Point &b) {
        return utils::equal(a.x, b.x) && utils::equal(a.y, b.y) && utils::equal(a.z, b.z);
    }

    // The triangle held by the index-th child of a group ("first child of g" is index 0)
    const Triangle &child_triangle(const Geom &group, const std::size_t index) {
        return std::get<Triangle>(std::get<Group>(group.shape).children.at(index)->shape);
    }

    bool group_includes(const Geom &group, const Geom *child) {
        const auto &children = std::get<Group>(group.shape).children;
        return std::ranges::find(children, child) != children.end();
    }
}

SCENARIO("Ignoring unrecognized lines") {
    GIVEN("A file containing gibberish") {
        constexpr std::string_view gibberish{
            "There was a young lady named Bright\n"
            "who traveled much faster than light.\n"
            "She set out one day\n"
            "in a relative way,\n"
            "and came back the previous night.\n"
        };
        WHEN("The file is parsed") {
            const auto parser = parse_obj_file(gibberish);
            THEN("The parser has ignored 5 lines") {
                REQUIRE(parser.ignored == 5);
            }
        }
    }
}

SCENARIO("Vertex records") {
    GIVEN("A file containing vertex records") {
        constexpr std::string_view file{
            "v -1 1 0\n"
            "v -1.0000 0.5000 0.0000\n"
            "v 1 0 0\n"
            "v 1 1 0\n"
        };
        WHEN("The file is parsed") {
            const auto parser = parse_obj_file(file);
            THEN("The vertices are recorded, numbered from 1") {
                REQUIRE(points_equal(parser.vertices[1], Point(-1, 1, 0)));
                REQUIRE(points_equal(parser.vertices[2], Point(-1, 0.5, 0)));
                REQUIRE(points_equal(parser.vertices[3], Point(1, 0, 0)));
                REQUIRE(points_equal(parser.vertices[4], Point(1, 1, 0)));
            }
        }
    }
}

SCENARIO("Parsing triangle faces") {
    GIVEN("A file containing vertices and triangle faces") {
        constexpr std::string_view file{
            "v -1 1 0\n"
            "v -1 0 0\n"
            "v 1 0 0\n"
            "v 1 1 0\n"
            "\n"
            "f 1 2 3\n"
            "f 1 3 4\n"
        };
        WHEN("The file is parsed") {
            const auto parser = parse_obj_file(file);
            const Geom &g = *parser.default_group;
            const Triangle &t1 = child_triangle(g, 0);
            const Triangle &t2 = child_triangle(g, 1);
            THEN("The default group holds a triangle per face, built from the indexed vertices") {
                REQUIRE(points_equal(t1.v1, parser.vertices[1]));
                REQUIRE(points_equal(t1.v2, parser.vertices[2]));
                REQUIRE(points_equal(t1.v3, parser.vertices[3]));
                REQUIRE(points_equal(t2.v1, parser.vertices[1]));
                REQUIRE(points_equal(t2.v2, parser.vertices[3]));
                REQUIRE(points_equal(t2.v3, parser.vertices[4]));
            }
        }
    }
}

SCENARIO("Triangulating polygons") {
    GIVEN("A file containing a pentagon face") {
        constexpr std::string_view file{
            "v -1 1 0\n"
            "v -1 0 0\n"
            "v 1 0 0\n"
            "v 1 1 0\n"
            "v 0 2 0\n"
            "\n"
            "f 1 2 3 4 5\n"
        };
        WHEN("The file is parsed") {
            const auto parser = parse_obj_file(file);
            const Geom &g = *parser.default_group;
            const Triangle &t1 = child_triangle(g, 0);
            const Triangle &t2 = child_triangle(g, 1);
            const Triangle &t3 = child_triangle(g, 2);
            THEN("The polygon is split into a fan of triangles sharing the first vertex") {
                REQUIRE(points_equal(t1.v1, parser.vertices[1]));
                REQUIRE(points_equal(t1.v2, parser.vertices[2]));
                REQUIRE(points_equal(t1.v3, parser.vertices[3]));
                REQUIRE(points_equal(t2.v1, parser.vertices[1]));
                REQUIRE(points_equal(t2.v2, parser.vertices[3]));
                REQUIRE(points_equal(t2.v3, parser.vertices[4]));
                REQUIRE(points_equal(t3.v1, parser.vertices[1]));
                REQUIRE(points_equal(t3.v2, parser.vertices[4]));
                REQUIRE(points_equal(t3.v3, parser.vertices[5]));
            }
        }
    }
}

SCENARIO("Triangles in groups") {
    GIVEN("The file triangles.obj") {
        const std::string file = read_file(FEATURES_DIR "/triangles.obj");
        WHEN("The file is parsed") {
            const auto parser = parse_obj_file(file);
            const Geom *g1 = parser.group("FirstGroup");
            const Geom *g2 = parser.group("SecondGroup");
            THEN("Each face is in the group named before it") {
                REQUIRE(g1 != nullptr);
                REQUIRE(g2 != nullptr);
                const Triangle &t1 = child_triangle(*g1, 0);
                const Triangle &t2 = child_triangle(*g2, 0);
                REQUIRE(points_equal(t1.v1, parser.vertices[1]));
                REQUIRE(points_equal(t1.v2, parser.vertices[2]));
                REQUIRE(points_equal(t1.v3, parser.vertices[3]));
                REQUIRE(points_equal(t2.v1, parser.vertices[1]));
                REQUIRE(points_equal(t2.v2, parser.vertices[3]));
                REQUIRE(points_equal(t2.v3, parser.vertices[4]));
            }
        }
    }
}

SCENARIO("Converting an OBJ file to a group") {
    GIVEN("The parsed file triangles.obj") {
        const std::string file = read_file(FEATURES_DIR "/triangles.obj");
        auto parser = parse_obj_file(file);
        WHEN("The parser is converted to a group") {
            const Geom &g = obj_to_group(parser);
            THEN("The group includes both named groups") {
                REQUIRE(group_includes(g, parser.group("FirstGroup")));
                REQUIRE(group_includes(g, parser.group("SecondGroup")));
            }
        }
    }
}
