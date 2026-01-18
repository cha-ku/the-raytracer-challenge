add_executable(tests
        test_main.cpp
        test_transform.cpp
        test_ray_sphere_intersection.cpp
)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain matrix)

# Add custom target to build all tests
add_custom_target(all_tests DEPENDS tests)

include(CTest)
include(Catch)
catch_discover_tests(tests)