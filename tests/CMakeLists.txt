# Download and build GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(memory_tests
    linear_allocator_test.cpp
)

target_link_libraries(memory_tests
    PRIVATE
        nova_memory
        gtest
        gtest_main
)

# Add test to CTest
add_test(NAME memory_tests COMMAND memory_tests)
