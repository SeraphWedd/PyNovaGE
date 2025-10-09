# Engine components CMake configuration

# Add main engine components
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/foundation/CMakeLists.txt")
    add_subdirectory(foundation)
endif()

# These will be added as they are implemented
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/CMakeLists.txt")
    add_subdirectory(core)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/systems/CMakeLists.txt")
    add_subdirectory(systems)
endif()

# Create interface library for the entire engine
add_library(engine INTERFACE)
add_library(PyNovaGE::Engine ALIAS engine)

# Find all built components and link them
file(GLOB_RECURSE COMPONENT_TARGETS 
    LIST_DIRECTORIES true 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/*/cmake_install.cmake"
)

foreach(target ${COMPONENT_TARGETS})
    if(TARGET ${target})
        target_link_libraries(engine INTERFACE ${target})
    endif()
endforeach()