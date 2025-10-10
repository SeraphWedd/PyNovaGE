# Voxel Demo Example
cmake_minimum_required(VERSION 3.15)

# Create voxel demo executable
add_executable(voxel_demo
    main.cpp
)

# Link required libraries
target_link_libraries(voxel_demo PRIVATE
    renderer
    window
    memory
    glfw
)

# Set output directory
set_target_properties(voxel_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)

# Copy shaders to output directory
add_custom_command(TARGET voxel_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/engine/core/renderer/shaders/voxel
        ${CMAKE_BINARY_DIR}/examples/shaders/voxel
)