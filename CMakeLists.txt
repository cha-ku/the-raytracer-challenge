cmake_minimum_required(VERSION 3.26)
project(the_raytracer_challenge)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use Clang and libc++ only if not already set by environment or command line
if(NOT DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "/usr/bin/clang")
endif()
if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif()

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

add_library(simulation STATIC src/simulation.cpp
    include/simulation.hpp
    include/Canvas.cpp)
target_include_directories(simulation PUBLIC include)

add_library(matrix STATIC
    include/Matrix.cpp
    include/Matrix.hpp
    include/MatrixImpl.hpp
    include/Point.hpp
    include/Utils.hpp
        include/Intersect.hpp
        include/Intersect.cpp
)
target_include_directories(matrix PUBLIC include)

add_executable(raytracer src/main.cpp
    include/Canvas.cpp)
target_include_directories(raytracer PUBLIC include)
target_link_libraries(raytracer simulation matrix)
