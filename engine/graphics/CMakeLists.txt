# Graphics Module
# Contains rendering systems including particles

# Add particle system
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/particles/CMakeLists.txt")
    add_subdirectory(particles)
endif()

# Create graphics module interface
add_library(graphics_module INTERFACE)
add_library(PyNovaGE::Graphics ALIAS graphics_module)

# Link particle system if available
if(TARGET particle_system)
    target_link_libraries(graphics_module INTERFACE particle_system)
endif()