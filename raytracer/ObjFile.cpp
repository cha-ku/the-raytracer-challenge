//
// Created by chaku on 16/11/2026.
//

#include "ObjFile.hpp"

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

        // `v x y z`: false if the record is malformed
        bool parse_vertex(ObjParser &parser, const std::vector<std::string_view> &words) {
            if (words.size() < 4) {
                return false;
            }
            const auto x{parse_number<float>(words[1])};
            const auto y{parse_number<float>(words[2])};
            const auto z{parse_number<float>(words[3])};
            if (!x || !y || !z) {
                return false;
            }
            parser.vertices.push_back(Point{.x = *x, .y = *y, .z = *z});
            return true;
        }

        // `f i j k ...`: fan-triangulates the polygon into `group`. false if the record is malformed.
        bool parse_face(ObjParser &parser, Geom &group, const std::vector<std::string_view> &words) {
            if (words.size() < 4) {
                return false;
            }
            std::vector<std::size_t> indices;
            indices.reserve(words.size() - 1);
            for (const std::string_view word: words | std::views::drop(1)) {
                // "v", "v/vt", "v//vn" or "v/vt/vn": the vertex index comes first
                const auto index{parse_number<std::size_t>(word.substr(0, word.find('/')))};
                if (!index || *index == 0 || *index >= parser.vertices.size()) {
                    return false;
                }
                indices.push_back(*index);
            }
            // Every triangle shares the first vertex: (1, 2, 3), (1, 3, 4), (1, 4, 5), ...
            for (std::size_t i{1}; i + 1 < indices.size(); ++i) {
                Geom &triangle{
                    new_geom(parser, Triangle{
                                 parser.vertices[indices[0]], parser.vertices[indices[i]],
                                 parser.vertices[indices[i + 1]]
                             })
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
                recognised = parse_vertex(parser, words);
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
        return result;
    }
}
