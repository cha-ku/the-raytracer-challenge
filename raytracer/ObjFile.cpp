//
// Created by chaku on 16/11/2026.
//

#include "ObjFile.hpp"
#include "Intersect.hpp"

#include <charconv>
#include <fstream>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <system_error>

namespace raytracer {
    namespace {
        // Splits a line into its whitespace-separated words. The views point into `line`.
        std::vector<std::string_view> split_words(const std::string_view line) {
            std::vector<std::string_view> words;
            std::size_t start{0};
            while (true) {
                start = line.find_first_not_of(" \t", start);
                if (start == std::string_view::npos) {
                    return words;
                }
                const std::size_t end{line.find_first_of(" \t", start)};
                words.push_back(line.substr(start, end - start));
                if (end == std::string_view::npos) {
                    return words;
                }
                start = end;
            }
        }

        // Parses the whole word as a T (float or index); nullopt if it isn't one
        template<typename T>
        std::optional<T> parse_number(const std::string_view word) {
            T value{};
            const auto [end, error]{std::from_chars(word.data(), word.data() + word.size(), value)};
            if (error != std::errc{} || end != word.data() + word.size()) {
                return std::nullopt;
            }
            return value;
        }

        Geom &new_geom(ObjParser &parser, AnyShape shape) {
            return *parser.geoms.emplace_back(std::make_unique<Geom>(std::move(shape)));
        }

        template<typename T>
        bool parse_xyz(std::vector<T> &out, const std::vector<std::string_view> &words) {
            if (words.size() < 4) {
                return false;
            }
            using ElementType = decltype(T::x);
            const auto x{parse_number<ElementType>(words[1])};
            const auto y{parse_number<ElementType>(words[2])};
            const auto z{parse_number<ElementType>(words[3])};
            if (!x || !y || !z) {
                return false;
            }
            out.push_back(T{.x = *x, .y = *y, .z = *z});
            return true;
        }

        // One corner of a face: its vertex index and, if the face gives one, its vertex normal index
        struct FaceVertex {
            std::size_t vertex;
            std::optional<std::size_t> normal;
        };

        // A face word: "v", "v/vt", "v//vn" or "v/vt/vn". The texture index is ignored, so it isn't validated.
        // nullopt if the vertex or normal index is malformed or doesn't refer to an earlier record.
        std::optional<FaceVertex> parse_face_vertex(const ObjParser &parser, const std::string_view word) {
            const std::size_t first_slash{word.find('/')};
            const auto vertex{parse_number<std::size_t>(word.substr(0, first_slash))};
            if (!vertex || *vertex == 0 || *vertex >= parser.vertices.size()) {
                return std::nullopt;
            }
            // "v" or "v/vt": no normal
            const std::size_t second_slash{
                first_slash == std::string_view::npos ? first_slash : word.find('/', first_slash + 1)
            };
            if (second_slash == std::string_view::npos) {
                return FaceVertex{.vertex = *vertex, .normal = std::nullopt};
            }
            const auto normal{parse_number<std::size_t>(word.substr(second_slash + 1))};
            if (!normal || *normal == 0 || *normal >= parser.normals.size()) {
                return std::nullopt;
            }
            return FaceVertex{.vertex = *vertex, .normal = *normal};
        }

        // `f i j k ...`: fan-triangulates the polygon into `group`. Smooth triangles if every corner has a normal,
        // flat ones otherwise. false if the record is malformed.
        bool parse_face(ObjParser &parser, Geom &group, const std::vector<std::string_view> &words) {
            if (words.size() < 4) {
                return false;
            }
            std::vector<FaceVertex> corners;
            corners.reserve(words.size() - 1);
            bool smooth{true};
            for (const std::string_view word: words | std::views::drop(1)) {
                const auto corner{parse_face_vertex(parser, word)};
                if (!corner) {
                    return false;
                }
                smooth = smooth && corner->normal.has_value();
                corners.push_back(*corner);
            }
            // Every triangle shares the first corner: (1, 2, 3), (1, 3, 4), (1, 4, 5), ...
            const FaceVertex &first{corners[0]};
            for (std::size_t i{1}; i + 1 < corners.size(); ++i) {
                const FaceVertex &second{corners[i]};
                const FaceVertex &third{corners[i + 1]};
                const Point &p1{parser.vertices[first.vertex]};
                const Point &p2{parser.vertices[second.vertex]};
                const Point &p3{parser.vertices[third.vertex]};
                Geom &triangle{
                    smooth
                        ? new_geom(parser, SmoothTriangle{
                                       p1, p2, p3, parser.normals[*first.normal], parser.normals[*second.normal],
                                       parser.normals[*third.normal]
                                   })
                        : new_geom(parser, Triangle{p1, p2, p3})
                };
                add_child(group, triangle);
            }
            return true;
        }
    }

    Geom *ObjParser::group(const std::string_view name) const {
        const auto found{groups.find(name)};
        return found != groups.end() ? found->second : nullptr;
    }

    ObjParser parse_obj_file(const std::string_view contents) {
        ObjParser parser;
        parser.default_group = &new_geom(parser, Group{});
        Geom *current_group{parser.default_group};

        for (const auto line_range: contents | std::views::split('\n')) {
            std::string_view line{line_range};
            // Tolerate Windows line endings
            if (line.ends_with('\r')) {
                line.remove_suffix(1);
            }
            const std::vector<std::string_view> words{split_words(line)};
            // Blank lines are neither records nor gibberish
            if (words.empty()) {
                continue;
            }

            bool recognised{false};
            if (words[0] == "v") {
                recognised = parse_xyz(parser.vertices, words);
            } else if (words[0] == "vn") {
                recognised = parse_xyz(parser.normals, words);
            } else if (words[0] == "f") {
                recognised = parse_face(parser, *current_group, words);
            } else if (words[0] == "g" && words.size() >= 2) {
                const auto [it, inserted]{parser.groups.try_emplace(std::string{words[1]}, nullptr)};
                // A name seen again continues the same group
                if (inserted) {
                    it->second = &new_geom(parser, Group{});
                }
                current_group = it->second;
                recognised = true;
            }
            if (!recognised) {
                ++parser.ignored;
            }
        }
        return parser;
    }

    std::string read_file(const std::string &path) {
        std::ifstream file{path, std::ios::binary};
        if (!file) {
            throw std::runtime_error("Could not open " + path);
        }
        return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
    }

    Geom &obj_to_group(ObjParser &parser) {
        Geom &result{new_geom(parser, Group{})};
        add_child(result, *parser.default_group);
        for (Geom *named: parser.groups | std::views::values) {
            add_child(result, *named);
        }
        // The triangles never move after parsing, so their boxes can be cached now
        refresh_bounds(result);
        return result;
    }
}
