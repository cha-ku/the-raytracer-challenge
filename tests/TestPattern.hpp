#ifndef THE_RAYTRACER_CHALLENGE_TEST_PATTERN_HPP
#define THE_RAYTRACER_CHALLENGE_TEST_PATTERN_HPP

#include "Pattern.hpp"

namespace raytracer {
    // A minimal concrete Pattern used only to exercise Pattern's own behavior
    // (transform, and the world → object → pattern space conversion in
    // pattern_at_shape) in tests, without needing a real pattern like
    // StripedPattern. Its colour is just the pattern-space point it was given,
    // so tests can see exactly which point pattern_at received. Kept out of
    // Pattern.hpp (and AnyPattern) so production code can never use it.
    struct TestPattern : Pattern {
        bool operator==(const TestPattern &) const = default;
    };

    inline TestPattern test_pattern() { return TestPattern{}; }

    inline Colour pattern_at(const TestPattern &pattern, const Point &pattern_point) {
        return Colour{.r = pattern_point.x, .g = pattern_point.y, .b = pattern_point.z};
    }
}

#endif //THE_RAYTRACER_CHALLENGE_TEST_PATTERN_HPP
