# Scene System CMake Configuration
cmake_minimum_required(VERSION 3.20)

message(STATUS "Configuring Scene System...")

# Create scene system library
add_library(scene)

# Set target properties
set_target_properties(scene PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Add alias for consistent naming
add_library(PyNovaGE::Scene ALIAS scene)

# Include directories
target_include_directories(scene 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Source files - automatically detected
file(GLOB_RECURSE SCENE_SOURCES 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE SCENE_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# Add source files to target
target_sources(scene PRIVATE ${SCENE_SOURCES})

# Link dependencies
target_link_libraries(scene 
    PUBLIC
        math    # For Vector2f, Matrix3f, etc.
    PRIVATE
        # Additional private dependencies can be added here
)

# Optional dependencies based on what's available
if(TARGET PyNovaGE::Physics)
    target_link_libraries(scene PUBLIC PyNovaGE::Physics)
    target_compile_definitions(scene PRIVATE PYNOVAGE_HAS_PHYSICS=1)
endif()

if(TARGET PyNovaGE::Renderer)
    target_link_libraries(scene PUBLIC PyNovaGE::Renderer)
    target_compile_definitions(scene PRIVATE PYNOVAGE_HAS_RENDERER=1)
endif()

if(TARGET particle_system)  # Particles use a different naming scheme
    target_link_libraries(scene PUBLIC particle_system)
    target_compile_definitions(scene PRIVATE PYNOVAGE_HAS_PARTICLES=1)
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(scene PRIVATE /W4)
    # Disable specific MSVC warnings for this module if needed
    target_compile_definitions(scene PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(scene PRIVATE -Wall -Wextra -pedantic)
endif()

# Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(scene PRIVATE DEBUG=1 PYNOVAGE_DEBUG=1)
endif()

# Tests
if(PYNOVAGE_BUILD_TESTS)
    message(STATUS "Configuring Scene System tests...")
    add_subdirectory(tests)
endif()

# Print summary
list(LENGTH SCENE_SOURCES source_count)
list(LENGTH SCENE_HEADERS header_count)
message(STATUS "Scene System configured:")
message(STATUS "  - ${source_count} source files")
message(STATUS "  - ${header_count} header files")
message(STATUS "  - Tests: ${PYNOVAGE_BUILD_TESTS}")

# Integration check
if(TARGET math)
    message(STATUS "  - Math integration: OK")
else()
    message(WARNING "  - Math integration: MISSING (math target not found)")
endif()

if(TARGET PyNovaGE::Physics)
    message(STATUS "  - Physics integration: OK") 
else()
    message(STATUS "  - Physics integration: Not available")
endif()

if(TARGET PyNovaGE::Renderer)
    message(STATUS "  - Renderer integration: OK")
else()
    message(STATUS "  - Renderer integration: Not available")
endif()

if(TARGET particle_system)
    message(STATUS "  - Particles integration: OK")
else()
    message(STATUS "  - Particles integration: Not available")
endif()

message(STATUS "Scene System configuration complete.")