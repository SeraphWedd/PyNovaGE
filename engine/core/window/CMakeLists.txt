# Window module CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Collect source files
file(GLOB_RECURSE WINDOW_SOURCES 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE WINDOW_HEADERS 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

# Add glad
set(GLAD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/glad/src/gl.c
)

# Create window library
add_library(window ${WINDOW_SOURCES} ${WINDOW_HEADERS} ${GLAD_SOURCES})
add_library(PyNovaGE::Window ALIAS window)

# Set target properties
set_target_properties(window PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Include directories
target_include_directories(window
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/glad/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Find and link GLFW
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    # Fetch GLFW if not found
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
        GIT_SHALLOW TRUE
    )
    
    # Configure GLFW options
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(glfw)
    target_link_libraries(window PRIVATE glfw)  # Changed to PRIVATE
else()
    target_link_libraries(window PUBLIC glfw3::glfw3)
endif()

# Link foundation dependencies
target_link_libraries(window 
    PUBLIC 
        PyNovaGE::Math
    PRIVATE 
        PyNovaGE::Memory
)

# Platform-specific dependencies
if(WIN32)
    target_link_libraries(window PRIVATE opengl32)
    target_compile_definitions(window PRIVATE PYNOVAGE_PLATFORM_WINDOWS)
elseif(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    target_link_libraries(window PRIVATE ${OPENGL_LIBRARY} ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
    target_compile_definitions(window PRIVATE PYNOVAGE_PLATFORM_MACOS)
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(window PRIVATE OpenGL::GL)
    target_compile_definitions(window PRIVATE PYNOVAGE_PLATFORM_LINUX)
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(window PRIVATE /W4)
    target_compile_definitions(window PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(window PRIVATE -Wall -Wextra -pedantic)
endif()

# Tests
if(PYNOVAGE_BUILD_TESTS)
    file(GLOB_RECURSE WINDOW_TEST_SOURCES 
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )
    
    if(WINDOW_TEST_SOURCES)
        add_executable(window_tests ${WINDOW_TEST_SOURCES})
        target_link_libraries(window_tests 
            PRIVATE 
                window
                gtest_main
        )
        
        # Add to test suite
        add_test(NAME WindowTests COMMAND window_tests)
        
        # Install tests if requested
        if(PYNOVAGE_INSTALL_TESTS)
            install(TARGETS window_tests
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
            )
        endif()
    endif()
endif()

# Benchmarks
if(PYNOVAGE_BUILD_BENCHMARKS)
    file(GLOB_RECURSE WINDOW_BENCHMARK_SOURCES 
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cpp"
    )
    
    if(WINDOW_BENCHMARK_SOURCES)
        add_executable(window_benchmarks ${WINDOW_BENCHMARK_SOURCES})
        target_link_libraries(window_benchmarks 
            PRIVATE 
                window
                benchmark::benchmark_main
        )
        
        # Install benchmarks if requested
        if(PYNOVAGE_INSTALL_BENCHMARKS)
            install(TARGETS window_benchmarks
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/benchmarks
            )
        endif()
    endif()
endif()

# Export and install (temporarily commented due to GLFW export issue)
# TODO: Fix GLFW export issue and re-enable
# install(TARGETS window
#     EXPORT PyNovaGETargets
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )

# install(DIRECTORY include/
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#     FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
# )
