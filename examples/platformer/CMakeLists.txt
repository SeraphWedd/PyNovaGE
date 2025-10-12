cmake_minimum_required(VERSION 3.20)

project(platformer_example)

# Source files
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/player.cpp
    src/platform.cpp
)

# Find OpenAL package
set(OPENAL_DIR "${CMAKE_SOURCE_DIR}/third_party/openal")
set(OPENAL_INCLUDE_DIR "${OPENAL_DIR}/include")
set(OPENAL_LIBRARY "${OPENAL_DIR}/libs/Win64/OpenAL32.lib")

# Add executable
add_executable(platformer_example ${SOURCES})

# Link with PyNovaGE libraries
target_link_libraries(platformer_example PRIVATE
    foundation
    window
    renderer
    physics
    scene
    audio
    particle_system
)

# Include directories
target_include_directories(platformer_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/engine/foundation/math/include
    ${CMAKE_SOURCE_DIR}/engine/foundation/memory/include
    ${CMAKE_SOURCE_DIR}/engine/core/window/include
    ${CMAKE_SOURCE_DIR}/engine/core/renderer/include
    ${CMAKE_SOURCE_DIR}/engine/core/physics/include
    ${CMAKE_SOURCE_DIR}/engine/core/scene/include
    ${CMAKE_SOURCE_DIR}/engine/systems/audio/include
    ${CMAKE_SOURCE_DIR}/engine/systems/asset/include
    ${CMAKE_SOURCE_DIR}/engine/graphics/particles/include
    ${CMAKE_SOURCE_DIR}/build/_deps/glfw-src/include
    ${OPENAL_INCLUDE_DIR}
)

# Set output directory to bin
set_target_properties(platformer_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# Copy assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
