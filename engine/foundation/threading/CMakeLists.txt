# Threading component CMake configuration

# Collect source files automatically
file(GLOB_RECURSE THREADING_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE THREADING_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Add threading library
add_library(threading STATIC ${THREADING_SOURCES} ${THREADING_HEADERS})
add_library(PyNovaGE::Threading ALIAS threading)

# Configure include directories
target_include_directories(threading PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link with system threading library
find_package(Threads REQUIRED)
target_link_libraries(threading PUBLIC Threads::Threads)

# Require C++17
target_compile_features(threading PUBLIC cxx_std_17)

# Compiler-specific options for performance
if(MSVC)
    target_compile_options(threading PRIVATE /W4)
    target_compile_options(threading PRIVATE $<$<CONFIG:Release>:/O2 /Ob2>)
else()
    target_compile_options(threading PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(threading PRIVATE $<$<CONFIG:Release>:-O3 -march=native>)
endif()

# Export and install the target
install(TARGETS threading
    EXPORT PyNovaGETargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
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
    
    add_executable(threading_tests ${TEST_SOURCES})
    target_link_libraries(threading_tests PRIVATE threading GTest::gtest GTest::gtest_main)
    
    # Add tests to CTest
    gtest_discover_tests(threading_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
        PROPERTIES LABELS "Threading"
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
    
    add_executable(threading_benchmarks ${BENCH_SOURCES})
    target_link_libraries(threading_benchmarks PRIVATE threading benchmark::benchmark benchmark::benchmark_main)
endif()
