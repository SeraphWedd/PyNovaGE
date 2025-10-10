# Renderer Module
cmake_minimum_required(VERSION 3.20)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Collect source files automatically
file(GLOB_RECURSE RENDERER_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE RENDERER_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# GLAD OpenGL loader
set(GLAD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/glad/src/gl.c
)

# Renderer library
add_library(renderer STATIC ${RENDERER_SOURCES} ${GLAD_SOURCES})
add_library(PyNovaGE::Renderer ALIAS renderer)

# Configure include directories
target_include_directories(renderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/glad/include>
)

# Set target properties
target_compile_features(renderer PUBLIC cxx_std_20)

target_link_libraries(renderer PUBLIC
    math
    memory
    window
    ${OPENGL_LIBRARIES}
)

# Platform-specific OpenGL linking
if(WIN32)
    target_link_libraries(renderer PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(renderer PRIVATE "-framework OpenGL")
else()
    target_link_libraries(renderer PRIVATE GL)
endif()

# Compiler-specific settings
if(MSVC)
    target_compile_options(renderer PRIVATE /W4)
else()
    target_compile_options(renderer PRIVATE -Wall -Wextra -pedantic)
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
        add_executable(renderer_tests ${TEST_SOURCES})
        target_link_libraries(renderer_tests PRIVATE renderer GTest::gtest GTest::gtest_main)
        
        # Add tests to CTest
        gtest_discover_tests(renderer_tests
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
            PROPERTIES LABELS "Renderer"
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
        add_executable(renderer_benchmarks ${BENCH_SOURCES})
        target_link_libraries(renderer_benchmarks PRIVATE renderer benchmark::benchmark benchmark::benchmark_main)
    endif()
endif()

# Export and install the target (temporarily disabled due to window dependency export issue)
# TODO: Re-enable when window library export is fixed
# install(TARGETS renderer
#     EXPORT PyNovaGETargets
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
