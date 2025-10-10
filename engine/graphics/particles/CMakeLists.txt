# Particle System Module
cmake_minimum_required(VERSION 3.20)

# Collect source files automatically
file(GLOB_RECURSE PARTICLE_SYSTEM_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE PARTICLE_SYSTEM_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Create particle system library
add_library(particle_system STATIC ${PARTICLE_SYSTEM_SOURCES})
add_library(PyNovaGE::Particles ALIAS particle_system)

# Configure include directories
target_include_directories(particle_system
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set target properties
target_compile_features(particle_system PUBLIC cxx_std_20)

# Link required dependencies
target_link_libraries(particle_system
    PUBLIC
        math
        memory
        renderer
)

# Add compiler-specific options
if(MSVC)
    target_compile_options(particle_system PRIVATE /W4)
else()
    target_compile_options(particle_system PRIVATE -Wall -Wextra -Wpedantic)
endif()

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
        add_executable(particle_system_tests ${TEST_SOURCES})
        target_link_libraries(particle_system_tests PRIVATE particle_system GTest::gtest GTest::gtest_main)
        
        # Add tests to CTest
        gtest_discover_tests(particle_system_tests
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
            PROPERTIES LABELS "Particles"
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
    
    if(BENCH_SOURCES)
        add_executable(particle_system_benchmarks ${BENCH_SOURCES})
        target_link_libraries(particle_system_benchmarks PRIVATE particle_system benchmark::benchmark benchmark::benchmark_main)
    endif()
endif()

# Export and install the target (temporarily disabled due to renderer dependency export issue)
# TODO: Re-enable when renderer library export is fixed
# install(TARGETS particle_system
#     EXPORT PyNovaGETargets
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
