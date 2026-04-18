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

add_library(simulation STATIC raytracer/Simulation.cpp
        raytracer/Simulation.hpp
        raytracer/Canvas.cpp)
target_include_directories(simulation PUBLIC raytracer)

add_library(matrix STATIC
        raytracer/Matrix.cpp
        raytracer/Matrix.hpp
        raytracer/MatrixImpl.hpp
        raytracer/Point.hpp
        raytracer/Utils.hpp
        raytracer/Intersect.hpp
        raytracer/Intersect.cpp
)

target_include_directories(matrix PUBLIC raytracer)

add_library(scene STATIC
        raytracer/Light.hpp
        raytracer/Material.hpp
        raytracer/Material.cpp
        raytracer/World.hpp
        raytracer/World.cpp
        raytracer/Camera.cpp
        raytracer/Camera.hpp
)

target_include_directories(scene PUBLIC raytracer)
target_link_libraries(scene PUBLIC matrix)

add_executable(raytracer raytracer/main.cpp
        raytracer/Canvas.cpp
)

target_include_directories(raytracer PUBLIC raytracer)
target_link_libraries(raytracer simulation matrix scene)
