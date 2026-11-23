//
// Created by chaku on 16/11/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_OBJFILE_HPP
#define THE_RAYTRACER_CHALLENGE_OBJFILE_HPP

#include "Geom.hpp"
#include "Point.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace raytracer {
    // The result of parsing an OBJ file: its vertices and the triangles built from its faces, sorted into groups.
    //
    // The parser owns every Geom it creates (triangles and groups) through std::unique_ptr, the same way World
    // does, so the Geom* in `default_group`, `groups` and each group's children stay valid when the parser is
    // moved. It is move-only.
    struct ObjParser {
        // Lines that were not a recognised record (also counts recognised records that failed to parse)
        std::size_t ignored{0};

        // OBJ indices are 1-based, so vertices[0] is an unused placeholder and vertices[1] is the first vertex
        std::vector<Point> vertices{Point{}};

        std::vector<Vector> normals{Vector{}};

        // Owns every triangle and group created while parsing
        std::vector<std::unique_ptr<Geom>> geoms;

        // Faces before any `g` line go here
        Geom *default_group{nullptr};

        // Named groups from `g <name>` lines. std::less<> allows lookup by string_view without a temporary string.
        std::map<std::string, Geom *, std::less<>> groups;

        // The named group, or nullptr if the file has no group with that name
        [[nodiscard]] Geom *group(std::string_view name) const;
    };

    // Parses OBJ text. Recognises `v x y z` vertex records, `f i j k ...` faces (polygons are fan-triangulated;
    // only the vertex index before any '/' is used) and `g name` groups. Every other non-empty line is ignored.
    ObjParser parse_obj_file(std::string_view contents);

    // Reads a whole file into a string, to pass to parse_obj_file. Throws std::runtime_error if it can't be opened.
    std::string read_file(const std::string &path);

    // Creates a group (owned by the parser) holding the default group and every named group, and returns it
    Geom &obj_to_group(ObjParser &parser);
}

#endif //THE_RAYTRACER_CHALLENGE_OBJFILE_HPP
