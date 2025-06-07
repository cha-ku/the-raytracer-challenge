add_executable(tests
        test_main.cpp
)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)