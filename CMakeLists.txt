cmake_minimum_required(VERSION 3.26)
project(the_raytracer_challenge)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use Clang and libc++ only if not already set by environment or command line
if (NOT DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "/usr/bin/clang")
endif ()
if (NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif ()

# Use Clang's libc++ standard library
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/catch2.git
        GIT_TAG v3.8.1
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(tests)

# Compile all targets (except in tests) for this machine's CPU in Release builds. Turn off with -DRT_NATIVE=OFF for a
# portable binary, or to compare against one.
option(RT_NATIVE "Compile for this machine's CPU (-march=native) in Release builds" ON)
add_compile_options($<$<AND:$<CONFIG:Release>,$<BOOL:${RT_NATIVE}>>:-march=native>)

# Math functions (e.g. std::sqrt in the intersection code) don't set errno on bad inputs, so they compile to single
# instructions instead of a call plus an error check. Nothing here reads errno, and results are unchanged. Unlike
# -ffast-math, it keeps infinities and NaNs working, which the unbounded shapes and empty bounding boxes rely on.
add_compile_options(-fno-math-errno)

add_library(simulation STATIC raytracer/Simulation.cpp
        raytracer/Simulation.hpp
        raytracer/ClockTower.cpp
        raytracer/ClockTower.hpp
        raytracer/Canvas.cpp)
target_include_directories(simulation PUBLIC raytracer)
target_link_libraries(simulation PUBLIC objreader scene)

add_library(raytracer_math STATIC
        raytracer/Matrix.cpp
        raytracer/Matrix.hpp
        raytracer/MatrixImpl.hpp
        raytracer/Point.hpp
        raytracer/Utils.hpp
        raytracer/Ray.hpp
)

add_library(objreader STATIC
        raytracer/ObjFile.hpp
        raytracer/ObjFile.cpp)
target_include_directories(objreader PUBLIC raytracer)
target_link_libraries(objreader PUBLIC scene)

target_include_directories(raytracer_math PUBLIC raytracer)

add_library(scene STATIC
        raytracer/Intersect.hpp
        raytracer/Intersect.cpp
        raytracer/Geom.hpp
        raytracer/Geom.cpp
        raytracer/Light.hpp
        raytracer/Material.hpp
        raytracer/Material.cpp
        raytracer/World.hpp
        raytracer/World.cpp
        raytracer/Camera.cpp
        raytracer/Camera.hpp
        raytracer/Pattern.hpp
        raytracer/Pattern.cpp
)

target_include_directories(scene PUBLIC raytracer)
target_link_libraries(scene PUBLIC raytracer_math)

add_executable(raytracer raytracer/main.cpp)

target_include_directories(raytracer PUBLIC raytracer)
target_compile_options(raytracer PUBLIC -Wall -Wpedantic -Wextra -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-pre-c++20-compat-pedantic -Wno-unsafe-buffer-usage)
target_link_libraries(raytracer simulation raytracer_math scene)
