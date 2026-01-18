# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

This project uses CMake with presets. Configure and build with Ninja:

```bash
# Configure (debug)
cmake --preset debug

# Build (debug)
cmake --build --preset debug

# Configure and build (release)
cmake --preset release
cmake --build --preset release
```

## Running Tests

Tests use Catch2 and CTest:

```bash
# Run all tests
ctest --preset debug

# Run tests with output
ctest --preset debug --output-on-failure

# Run a specific test by name
./cmake-build-debug/tests/tests "[Translation test]"

# Run tests matching a pattern
./cmake-build-debug/tests/tests "Rotation*"
```

## Architecture

This is a raytracer implementation in C++23, following "The Ray Tracer Challenge" book.

### Core Types (all in `raytracer` namespace)

- **Point/Vector** (`include/Point.hpp`, `include/Vector.hpp`): 3D geometric primitives with w=1 for points, w=0 implicit for vectors. Support standard operations (add, subtract, dot, cross, normalize).

- **Matrix/Container** (`include/Matrix.hpp`, `include/MatrixImpl.hpp`): Matrix operations using `std::mdspan` for views over `Container<T>` storage. Supports multiply, transpose, determinant, inverse, and transformations (translation, scale, rotation_x/y/z, shearing).

- **Ray/Sphere/Intersection** (`include/Intersect.hpp`): Ray casting primitives. `intersect()` returns vector of intersections, `hit()` finds the visible intersection.

- **Colour/Canvas** (`include/Colour.hpp`, `include/Canvas.hpp`): RGB colors with Hadamard product; Canvas outputs to PPM format.

### Key Patterns

- Transformations are 4x4 matrices applied via `multiply(transform, make_container(point))`
- Floating-point comparisons use `utils::equal()` with epsilon tolerance
- Template implementations in `MatrixImpl.hpp` must be included where matrix operations are used

## Feature Files

Test specifications from "The Ray Tracer Challenge" book are in `tests/features/*.feature` (Gherkin format). When implementing new functionality, create Catch2 tests based on the scenarios in these files. Each `Scenario` becomes a `TEST_CASE` or `SECTION`, and `Given/When/Then` steps map to setup, action, and `REQUIRE` assertions.
