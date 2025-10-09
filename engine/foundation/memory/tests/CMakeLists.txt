# Memory Tests
find_package(GTest REQUIRED)

set(MEMORY_TEST_SOURCES
    test_allocator.cpp
    test_memory_pool.cpp
    test_stack_allocator.cpp
    test_object_pool.cpp
)

add_executable(memory_tests ${MEMORY_TEST_SOURCES})

target_link_libraries(memory_tests
    memory
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(memory_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Register tests with CTest
add_test(NAME memory_tests COMMAND memory_tests)