# Math library component
file(GLOB_RECURSE MATH_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# Create header-only library
add_library(Math INTERFACE)
target_include_directories(Math INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Tests
if(BUILD_TESTING)
    file(GLOB_RECURSE MATH_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )
    
    add_executable(Math_Tests ${MATH_TEST_SOURCES})
    target_link_libraries(Math_Tests PRIVATE
        Math
        GTest::gtest
        GTest::gtest_main
    )
    
    add_test(NAME Math_Tests COMMAND Math_Tests)
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    file(GLOB_RECURSE MATH_BENCHMARK_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cpp"
    )
    
    add_executable(Math_Benchmarks ${MATH_BENCHMARK_SOURCES})
    target_link_libraries(Math_Benchmarks PRIVATE
        Math
        benchmark::benchmark
        benchmark::benchmark_main
    )
endif()
