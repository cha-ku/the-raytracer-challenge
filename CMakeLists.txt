cmake_minimum_required(VERSION 3.26)
project(the_raytracer_challenge)
set(CMAKE_CXX_STANDARD 23)

Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/catch2.git
        GIT_TAG v3.8.1
)

FetchContent_MakeAvailable(Catch2)

add_subdirectory(tests)

add_executable(raytracer src/main.cpp)
target_include_directories(raytracer PUBLIC include)