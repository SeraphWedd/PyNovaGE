# Memory component CMake configuration


# Collect source files automatically
file(GLOB_RECURSE MEMORY_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE MEMORY_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Add memory library
add_library(memory STATIC ${MEMORY_SOURCES})
add_library(PyNovaGE::Memory ALIAS memory)

# Configure include directories
target_include_directories(memory
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set target properties
target_compile_features(memory PUBLIC cxx_std_20)

# Enable SIMD optimizations
if(MSVC)
    target_compile_options(memory PRIVATE /arch:AVX2)
else()
    target_compile_options(memory PRIVATE -mavx2 -mfma)
endif()

# Export and install the target
install(TARGETS memory
    EXPORT PyNovaGETargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


# Configure tests if enabled
if(PYNOVAGE_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    file(GLOB_RECURSE TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
    )
    
    add_executable(memory_tests ${TEST_SOURCES})
    target_link_libraries(memory_tests PRIVATE memory GTest::gtest GTest::gtest_main)
    
    # Add tests to CTest
    gtest_discover_tests(memory_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
        PROPERTIES LABELS "Memory"
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
    
    add_executable(memory_benchmarks ${BENCH_SOURCES})
    target_link_libraries(memory_benchmarks PRIVATE memory benchmark::benchmark benchmark::benchmark_main)
endif()
