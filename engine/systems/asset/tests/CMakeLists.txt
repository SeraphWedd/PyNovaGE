cmake_minimum_required(VERSION 3.15)

# Find test files
file(GLOB_RECURSE ASSET_TEST_SOURCES "*.cpp")

# Create test executable
add_executable(asset_tests ${ASSET_TEST_SOURCES})

# Link libraries
target_link_libraries(asset_tests
    asset
    gtest
    gtest_main
)

# Include directories
target_include_directories(asset_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/engine/systems/asset/include
)

# Add to test suite
if(PYNOVAGE_BUILD_TESTS)
    add_test(NAME asset_system_tests COMMAND asset_tests)
endif()

message(STATUS "Asset tests configured")