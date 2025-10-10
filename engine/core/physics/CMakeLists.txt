# Physics System CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Physics library
add_library(physics
    src/collision_shapes.cpp
    src/rigid_body.cpp
)

# Create alias for consistent naming
add_library(PyNovaGE::Physics ALIAS physics)

# Set target properties
set_target_properties(physics PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(physics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add math headers as interface include directory
target_include_directories(physics
    PUBLIC
        ${CMAKE_SOURCE_DIR}/engine/foundation/math/include
)

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

# Tests
if(BUILD_TESTING)
    # Physics unit tests
    add_executable(physics_tests
        tests/test_collision_shapes.cpp
    )
    
    target_link_libraries(physics_tests
        PRIVATE
            physics
            gtest
            gtest_main
    )
    
    # Copy test properties from physics library
    set_target_properties(physics_tests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
    
    # Add the same compiler optimizations to tests
    if(MSVC)
        target_compile_options(physics_tests PRIVATE
            /W4 /WX- /wd4702 /arch:AVX2 /fp:fast /Ob3 /favor:INTEL64
        )
    elseif(GCC OR Clang)
        target_compile_options(physics_tests PRIVATE
            -Wall -Wextra -march=native -O3 -ffast-math
        )
    endif()
    
    # Register the test
    include(GoogleTest)
    gtest_discover_tests(physics_tests)
    
    # Add custom test target
    add_custom_target(run_physics_tests
        COMMAND physics_tests
        DEPENDS physics_tests
        COMMENT "Running physics system unit tests"
    )
endif()

# Benchmarks
if(BUILD_BENCHMARKING)
    # Physics benchmarks  
    add_executable(physics_benchmarks
        tests/physics_benchmarks.cpp
    )
    
    target_link_libraries(physics_benchmarks
        PRIVATE
            physics
            benchmark
            benchmark_main
    )
    
    # Copy properties and optimizations
    set_target_properties(physics_benchmarks PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
    
    if(MSVC)
        target_compile_options(physics_benchmarks PRIVATE
            /W4 /WX- /wd4702 /arch:AVX2 /fp:fast /Ob3 /favor:INTEL64 /Qpar
        )
    elseif(GCC OR Clang) 
        target_compile_options(physics_benchmarks PRIVATE
            -Wall -Wextra -march=native -O3 -ffast-math -funroll-loops
        )
    endif()
    
    # Add custom benchmark target
    add_custom_target(run_physics_benchmarks
        COMMAND physics_benchmarks
        DEPENDS physics_benchmarks
        COMMENT "Running physics system benchmarks"
    )
endif()

# Install targets (if needed)
if(INSTALL_PYNOVAGE)
    install(TARGETS physics
        EXPORT PyNovaGETargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
    
    install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
    )
endif()

# Development helper targets
add_custom_target(physics_dev
    DEPENDS physics
    COMMENT "Building physics system"
)

if(BUILD_TESTING)
    add_dependencies(physics_dev physics_tests)
endif()

if(BUILD_BENCHMARKING) 
    add_dependencies(physics_dev physics_benchmarks)
endif()

# Print configuration info
message(STATUS "Physics system configuration:")
message(STATUS "  - SIMD optimizations: Enabled (AVX2/AVX/SSE2)")
message(STATUS "  - Tests: ${BUILD_TESTING}")
message(STATUS "  - Benchmarks: ${BUILD_BENCHMARKING}")
message(STATUS "  - Compiler optimizations: Enabled")