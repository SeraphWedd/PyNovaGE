# Audio System Tests
cmake_minimum_required(VERSION 3.20)

message(STATUS "Configuring Audio System tests...")

# Collect test source files
file(GLOB_RECURSE AUDIO_TEST_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Create test executable
add_executable(audio_tests ${AUDIO_TEST_SOURCES})

# Set target properties
set_target_properties(audio_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER "Tests/Systems"
)

# Include directories
target_include_directories(audio_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../include
        ../../asset/include
        ../../../foundation/math/include
        ${OPENAL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(audio_tests
    PRIVATE
        audio
        asset
        math
        gtest
        gtest_main
        ${OPENAL_LIBRARIES}
)

# Add test to CTest
add_test(NAME AudioSystemTests COMMAND audio_tests)

# Test configuration summary
message(STATUS "Audio System tests configured:")
message(STATUS "  - Test sources: ${AUDIO_TEST_SOURCES}")
message(STATUS "  - Executable: audio_tests")
message(STATUS "  - OpenAL integration: OK")
message(STATUS "Audio System tests configuration complete.")