# Math library CMake configuration
cmake_minimum_required(VERSION 3.15)

# Create the math library target
add_library(pynovage_math
    src/simd_utils.cpp
)

# Add include directories
target_include_directories(pynovage_math
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable SIMD instructions based on platform
if(MSVC)
    target_compile_options(pynovage_math PRIVATE /arch:SSE2)
else()
    target_compile_options(pynovage_math PRIVATE -msse2)
endif()

# Add test executable
enable_testing()

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

FetchContent_MakeAvailable(googletest googlebenchmark)

add_executable(math_tests
    tests/simd_utils_tests.cpp
    tests/vector2_tests.cpp
    tests/simd_utils_vec3_tests.cpp
    tests/vector3_tests.cpp
    tests/matrix2_tests.cpp
    tests/matrix3_tests.cpp
)

target_link_libraries(math_tests
    PRIVATE
        pynovage_math
        gtest
        gtest_main
)

target_include_directories(math_tests
    PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
)

add_test(NAME math_tests COMMAND math_tests)

# Add benchmark executable
add_executable(math_benchmarks
    tests/math_perf_tests.cpp
    tests/vector2_perf_tests.cpp
    tests/vector3_perf_tests.cpp
    tests/matrix_perf_tests.cpp
)

target_link_libraries(math_benchmarks
    PRIVATE
        pynovage_math
        benchmark::benchmark
        benchmark::benchmark_main
)

target_include_directories(math_benchmarks
    PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/googlebenchmark-src/include
)
