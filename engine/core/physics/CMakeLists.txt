# Physics System CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Collect source files automatically
file(GLOB_RECURSE PHYSICS_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE PHYSICS_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Physics library
add_library(physics STATIC ${PHYSICS_SOURCES})

# Create alias for consistent naming
add_library(PyNovaGE::Physics ALIAS physics)

# Set target properties
set_target_properties(physics PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Configure include directories
target_include_directories(physics
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(physics PUBLIC math)

# Compiler-specific optimizations
if(MSVC)
    target_compile_options(physics PRIVATE
        /W4               # High warning level (without treating warnings as errors)
        /arch:AVX2        # Enable AVX2 for SIMD optimizations
        /fp:fast          # Fast floating point model
        /Ob3              # Aggressive inlining
        /favor:INTEL64    # Optimize for Intel 64-bit
        /Qpar             # Auto-parallelization
        /Qvec-report:2    # Vectorization reporting
    )
elseif(GCC OR Clang)
    target_compile_options(physics PRIVATE
        -Wall -Wextra -Werror    # High warning level
        -march=native            # Enable all supported SIMD instructions
        -O3                      # Maximum optimization
        -ffast-math              # Fast math optimizations
        -funroll-loops           # Loop unrolling
    )
endif()

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(physics PRIVATE
        PYNOVAGE_PLATFORM_WINDOWS
        NOMINMAX                    # Prevent Windows.h min/max macros
        _CRT_SECURE_NO_WARNINGS    # Disable MSVC security warnings
    )
endif()

# SIMD feature detection is handled by the math library headers

# Configure tests if enabled
if(PYNOVAGE_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    file(GLOB_RECURSE TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
    )
    
    # Filter out benchmark files from tests
    list(FILTER TEST_SOURCES EXCLUDE REGEX ".*benchmarks?.*")
    
    if(TEST_SOURCES)
        add_executable(physics_tests ${TEST_SOURCES})
        target_link_libraries(physics_tests PRIVATE physics GTest::gtest GTest::gtest_main)
        
        # Add tests to CTest
        gtest_discover_tests(physics_tests
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
            PROPERTIES LABELS "Physics"
        )
    endif()
endif()

# Configure benchmarks if enabled
if(PYNOVAGE_BUILD_BENCHMARKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks")
    file(GLOB_RECURSE BENCH_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.h"
    )
    
    # Also check for benchmark files in tests directory
    file(GLOB_RECURSE TEST_BENCH_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*benchmarks*.cpp"
    )
    
    list(APPEND BENCH_SOURCES ${TEST_BENCH_SOURCES})
    
    if(BENCH_SOURCES)
        add_executable(physics_benchmarks ${BENCH_SOURCES})
        target_link_libraries(physics_benchmarks PRIVATE physics benchmark::benchmark benchmark::benchmark_main)
    endif()
endif()

# Export and install the target
install(TARGETS physics
    EXPORT PyNovaGETargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
