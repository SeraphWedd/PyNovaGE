# Scene System Tests CMake Configuration
cmake_minimum_required(VERSION 3.20)

message(STATUS "Configuring Scene System tests...")

# Test executable
add_executable(scene_tests)

# Set target properties
set_target_properties(scene_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Test source files
file(GLOB_RECURSE SCENE_TEST_SOURCES 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

target_sources(scene_tests PRIVATE ${SCENE_TEST_SOURCES})

# Link dependencies
target_link_libraries(scene_tests PRIVATE
    PyNovaGE::Scene
    gtest
    gtest_main
)

# Include directories for tests
target_include_directories(scene_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add test cases to CTest
if(CMAKE_TESTING_ENABLED OR PYNOVAGE_BUILD_TESTS)
    include(GoogleTest)
    gtest_discover_tests(scene_tests)
endif()

# Test summary
list(LENGTH SCENE_TEST_SOURCES test_count)
message(STATUS "Scene System tests configured:")
message(STATUS "  - ${test_count} test files")
message(STATUS "  - Integration with GoogleTest: OK")