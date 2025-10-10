cmake_minimum_required(VERSION 3.20)

project(texture_test)

# Create texture test executable
add_executable(texture_test
    main.cpp
)

# Link against required libraries
target_link_libraries(texture_test
    PRIVATE
    renderer
    window
    memory
    glfw
)

# Set properties
set_target_properties(texture_test
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# Set output directory
set_target_properties(texture_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)