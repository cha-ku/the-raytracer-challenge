add_executable(tests
        test_main.cpp
        test_transform.cpp
        test_ray_sphere_intersection.cpp
        test_lights.cpp
        test_materials.cpp
        test_world.cpp
        test_camera.cpp
        test_shapes.cpp
        test_planes.cpp
        test_patterns.cpp
        test_cubes.cpp
        test_cylinders.cpp
        test_cones.cpp
        test_groups.cpp
        test_triangles.cpp
        test_obj_file.cpp
)

target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/raytracer)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain matrix scene objreader)
# Lets tests open fixture files such as features/triangles.obj
target_compile_definitions(tests PRIVATE FEATURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/features")
#target_compile_options(tests PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-pre-c++20-compat-pedantic)

# Add custom target to build all tests
add_custom_target(all_tests DEPENDS tests)

include(CTest)
include(Catch)
catch_discover_tests(tests)