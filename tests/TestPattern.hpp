#ifndef THE_RAYTRACER_CHALLENGE_TEST_PATTERN_HPP
#define THE_RAYTRACER_CHALLENGE_TEST_PATTERN_HPP

#include "Pattern.hpp"

namespace raytracer {
    // The book's test_pattern(). TestPattern itself lives in Pattern.hpp (and AnyPattern) so it can be
    // assigned to a Material
    inline TestPattern test_pattern() { return TestPattern{}; }
}

#endif //THE_RAYTRACER_CHALLENGE_TEST_PATTERN_HPP
