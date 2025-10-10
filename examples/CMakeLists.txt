# Examples directory
cmake_minimum_required(VERSION 3.20)

# Only build examples if requested
option(BUILD_EXAMPLES "Build example applications" ON)

if(BUILD_EXAMPLES)
    message(STATUS "Building examples...")
    
    # Add example subdirectories
    add_subdirectory(basic_window)
    add_subdirectory(interactive_demo)
    add_subdirectory(shader_test)
    add_subdirectory(texture_test)
    add_subdirectory(sprite_test)
    add_subdirectory(voxel_demo)
    
    message(STATUS "Examples configured successfully")
else()
    message(STATUS "Skipping examples (BUILD_EXAMPLES=OFF)")
endif()