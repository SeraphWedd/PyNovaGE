# Math component CMake configuration


# Collect source files automatically
file(GLOB_RECURSE MATH_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE MATH_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Add math library as header-only for now
add_library(math INTERFACE)
add_library(PyNovaGE::Math ALIAS math)

# Configure include directories
target_include_directories(math INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compiler-specific options
if(MSVC)
    add_compile_options(/wd4702)  # Disable unreachable code warning
endif()

# Export and install the target
install(TARGETS math
    EXPORT PyNovaGETargets
    INCLUDES DESTINATION include
)


# Configure tests if enabled
if(PYNOVAGE_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    file(GLOB_RECURSE TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
    )
    
    add_executable(math_tests ${TEST_SOURCES})
    target_link_libraries(math_tests PRIVATE math GTest::gtest GTest::gtest_main)
    
    # Add tests to CTest
    gtest_discover_tests(math_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
        PROPERTIES LABELS "Math"
    )
endif()

# Configure benchmarks if enabled
if(PYNOVAGE_BUILD_BENCHMARKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks")
    file(GLOB_RECURSE BENCH_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.h"
    )
    
    add_executable(math_benchmarks ${BENCH_SOURCES})
    target_link_libraries(math_benchmarks PRIVATE math benchmark::benchmark benchmark::benchmark_main)
endif()