# PyNovaGE Python Bindings CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

# Set project name and version
project(pynovage_python)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pybind11
find_package(pybind11 REQUIRED)

# Include directories for engine headers
set(ENGINE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/core/window/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/core/renderer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/core/physics/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/core/scene/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/systems/asset/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/systems/audio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/foundation/math/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/foundation/core/include
)

# Collect all binding source files
file(GLOB_RECURSE BINDING_SOURCES "src/*.cpp")

# Create the Python module
pybind11_add_module(pynovage_core ${BINDING_SOURCES})

# Include directories
target_include_directories(pynovage_core PRIVATE ${ENGINE_INCLUDE_DIRS})

# Link against engine libraries (assuming they're built separately)
target_link_libraries(pynovage_core PRIVATE
    PyNovaGE_Window
    PyNovaGE_Renderer  
    PyNovaGE_Physics
    PyNovaGE_Scene
    PyNovaGE_Asset
    PyNovaGE_Audio
    PyNovaGE_Math
    PyNovaGE_Core
)

# Compiler-specific options
target_compile_definitions(pynovage_core PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})

# Platform-specific configurations
if(WIN32)
    target_link_libraries(pynovage_core PRIVATE opengl32 glfw3)
endif()

# Install the module
install(TARGETS pynovage_core DESTINATION .)