# Memory library CMake configuration
cmake_minimum_required(VERSION 3.15)

# Create the memory library target
add_library(pynovage_memory INTERFACE)

# Add include directories
target_include_directories(pynovage_memory
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable testing
enable_testing()

# Configure dependency fetching
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.2
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Configure benchmark options
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

# Set default benchmark runtime configuration
set(BENCHMARK_MIN_TIME "0.1s" CACHE STRING "Minimum time to run each benchmark")
set(BENCHMARK_ARGS "--benchmark_min_time=${BENCHMARK_MIN_TIME}" CACHE STRING "Arguments to pass to benchmark executables")

FetchContent_MakeAvailable(googletest googlebenchmark)

# Benchmarks
if(PYNOVAGE_BUILD_BENCHMARKS)
    # Linear allocator benchmarks
    add_executable(allocator_benchmarks_linear
        tests/allocator_perf_tests.cpp
        tests/memory_benchmarks.cpp
    )
target_link_libraries(allocator_benchmarks_linear
        PRIVATE
            pynovage_memory
            benchmark::benchmark
            benchmark::benchmark_main
    )

    # Pool allocator benchmarks
    add_executable(allocator_benchmarks_pool
        tests/pool_allocator_perf_tests.cpp
        tests/memory_benchmarks.cpp
    )
target_link_libraries(allocator_benchmarks_pool
        PRIVATE
            pynovage_memory
            benchmark::benchmark
            benchmark::benchmark_main
    )

    # Stack allocator benchmarks
    add_executable(allocator_benchmarks_stack
        tests/stack_allocator_perf_tests.cpp
        tests/memory_benchmarks.cpp
    )
target_link_libraries(allocator_benchmarks_stack
        PRIVATE
            pynovage_memory
            benchmark::benchmark
            benchmark::benchmark_main
    )

    # Defragmenting allocator benchmarks - split into separate executables
    add_executable(defrag_basic_benchmarks
        tests/basic_ops_main.cpp
    )
target_link_libraries(defrag_basic_benchmarks
        PRIVATE
            pynovage_memory
            benchmark::benchmark
            benchmark::benchmark_main
    )
    
    add_executable(defrag_threading_benchmarks
        tests/threading_main.cpp
    )
target_link_libraries(defrag_threading_benchmarks
        PRIVATE
            pynovage_memory
            benchmark::benchmark
            benchmark::benchmark_main
    )
    
    add_executable(defrag_sizeclass_benchmarks
        tests/sizeclass_main.cpp
    )
target_link_libraries(defrag_sizeclass_benchmarks
        PRIVATE
            pynovage_memory
            benchmark::benchmark
            benchmark::benchmark_main
    )
endif()

# Vector tests
add_executable(vector_tests
    tests/vector_tests.cpp
)
target_link_libraries(vector_tests
    PRIVATE
        pynovage_memory
        gtest
        gtest_main
)
add_test(NAME vector_tests COMMAND vector_tests)

# Defrag allocator tests
add_executable(defrag_allocator_tests
    tests/defrag_allocator_tests.cpp
)
target_link_libraries(defrag_allocator_tests
    PRIVATE
        pynovage_memory
        gtest
        gtest_main
)
add_test(NAME defrag_allocator_tests COMMAND defrag_allocator_tests)

# String tests
add_executable(string_tests
    tests/string_tests.cpp
)
target_link_libraries(string_tests
    PRIVATE
        pynovage_memory
        gtest
        gtest_main
)
add_test(NAME string_tests COMMAND string_tests)

# String view tests
add_executable(string_view_tests
    tests/string_view_tests.cpp
)
target_link_libraries(string_view_tests
    PRIVATE
        pynovage_memory
        gtest
        gtest_main
)
add_test(NAME string_view_tests COMMAND string_view_tests)

# UTF tests
add_executable(utf_tests
    tests/utf_tests.cpp
)
target_link_libraries(utf_tests
    PRIVATE
        pynovage_memory
        gtest
        gtest_main
)
add_test(NAME utf_tests COMMAND utf_tests)

# Memory analysis benchmarks
if(PYNOVAGE_BUILD_BENCHMARKS)
    add_executable(memory_analysis_benchmarks
        tests/memory_analysis_benchmarks.cpp
    )
target_link_libraries(memory_analysis_benchmarks
        PRIVATE
            pynovage_memory
            benchmark::benchmark
            benchmark::benchmark_main
    )
endif()

# Add benchmark test with proper arguments
add_test(NAME memory_benchmarks COMMAND allocator_benchmarks_linear ${BENCHMARK_ARGS})
