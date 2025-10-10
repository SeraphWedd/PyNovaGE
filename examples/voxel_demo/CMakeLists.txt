cmake_minimum_required(VERSION 3.20)

project(voxel_demo)

# Create voxel demo executable
add_executable(voxel_demo
    main.cpp
)

# Link against required libraries
target_link_libraries(voxel_demo
    PRIVATE
    renderer
    window
    memory
    math
    glfw
)

# Set properties
set_target_properties(voxel_demo
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# Set output directory
set_target_properties(voxel_demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
