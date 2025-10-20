# Camera component CMake configuration

# Collect source files automatically
file(GLOB_RECURSE CAMERA_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE CAMERA_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Add camera library
add_library(camera STATIC ${CAMERA_SOURCES} ${CAMERA_HEADERS})
add_library(PyNovaGE::Camera ALIAS camera)

# Configure include directories
target_include_directories(camera PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(camera PUBLIC
    math
)

# Camera system is now GLFW-independent

# Require C++17
target_compile_features(camera PUBLIC cxx_std_17)

# Compiler-specific options
if(MSVC)
    target_compile_options(camera PRIVATE /W4)
else()
    target_compile_options(camera PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Export and install the target (temporarily commented due to GLFW export issue)
# TODO: Fix GLFW export issue and re-enable
# install(TARGETS camera
#     EXPORT PyNovaGETargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     INCLUDES DESTINATION include
# )

# Configure tests if enabled
if(PYNOVAGE_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    file(GLOB_RECURSE TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
    )
    
    add_executable(camera_tests ${TEST_SOURCES})
    target_link_libraries(camera_tests PRIVATE camera GTest::gtest GTest::gtest_main)
    
    # Add tests to CTest
    gtest_discover_tests(camera_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
        PROPERTIES LABELS "Camera"
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
    
    add_executable(camera_benchmarks ${BENCH_SOURCES})
    target_link_libraries(camera_benchmarks PRIVATE camera benchmark::benchmark benchmark::benchmark_main)
endif()