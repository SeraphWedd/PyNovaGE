# Math library CMake configuration
cmake_minimum_required(VERSION 3.15)

# Create the math library target
# Add profiling option
if(PROFILE_BROADPHASE)
    add_compile_definitions(PROFILE_BROADPHASE)
endif()

add_library(pynovage_math
    src/simd_utils.cpp
    src/geometry/continuous_collision.cpp
    src/geometry/high_speed_collision.cpp
    src/geometry/collision_response.cpp
    src/geometry/broad_phase.cpp
    src/lighting/attenuation.cpp
    src/lighting/point_light.cpp
    src/lighting/directional_light.cpp
    src/lighting/spot_light.cpp
    src/lighting/light_transforms.cpp
    src/lighting/shadow_map.cpp
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

# Set default benchmark runtime configuration
set(BENCHMARK_MIN_TIME "0.1s" CACHE STRING "Minimum time to run each benchmark")
set(BENCHMARK_ARGS "--benchmark_min_time=${BENCHMARK_MIN_TIME}" CACHE STRING "Arguments to pass to benchmark executables")

FetchContent_MakeAvailable(googletest googlebenchmark)

add_executable(math_tests
    tests/simd_utils_tests.cpp
    tests/vector2_tests.cpp
    tests/simd_utils_vec3_tests.cpp
    tests/vector3_tests.cpp
    tests/vector4_tests.cpp
    tests/matrix2_tests.cpp
    tests/matrix3_tests.cpp
    tests/matrix4_tests.cpp
    tests/quaternion_tests.cpp
    tests/primitives_tests.cpp
    tests/geometry/intersection_tests.cpp
    tests/geometry/continuous_collision_tests.cpp
    tests/geometry/high_speed_collision_tests.cpp
    tests/geometry/collision_response_tests.cpp
    tests/geometry/broad_phase_tests.cpp
    tests/lighting/light_types_tests.cpp
    tests/lighting/attenuation_tests.cpp
    tests/lighting/point_light_tests.cpp
    tests/lighting/directional_light_tests.cpp
    tests/lighting/spot_light_tests.cpp
    tests/lighting/light_transforms_tests.cpp
    tests/lighting/shadow_map_tests.cpp
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
    tests/vector2_perf_tests.cpp
    tests/vector3_perf_tests.cpp
    tests/vector4_perf_tests.cpp
    tests/simd_utils_perf_tests.cpp
    tests/primitives_perf_tests.cpp
    tests/matrix2_perf_tests.cpp
    tests/matrix3_perf_tests.cpp
    tests/matrix4_perf_tests.cpp

    tests/geometry/intersection_perf_tests.cpp
    tests/geometry/continuous_collision_perf_tests.cpp
    tests/geometry/collision_response_perf_tests.cpp
    tests/geometry/broad_phase_perf_tests.cpp
    tests/lighting/point_light_perf_tests.cpp
    tests/lighting/directional_light_perf_tests.cpp
    tests/lighting/spot_light_perf_tests.cpp
    tests/lighting/attenuation_perf_tests.cpp
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

# Add benchmark test with proper arguments
add_test(NAME math_benchmarks COMMAND math_benchmarks ${BENCHMARK_ARGS})
