# Core engine systems CMake configuration

# Add core system components
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/window/CMakeLists.txt")
    add_subdirectory(window)
endif()

# These will be added as they are implemented
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/physics/CMakeLists.txt")
    add_subdirectory(physics)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/render/CMakeLists.txt")
    add_subdirectory(render)
endif()

# Create interface library for all core systems
add_library(core INTERFACE)
add_library(PyNovaGE::Core ALIAS core)

# Link available core components
if(TARGET window)
    target_link_libraries(core INTERFACE PyNovaGE::Window)
endif()

if(TARGET physics)
    target_link_libraries(core INTERFACE PyNovaGE::Physics)
endif()

if(TARGET render)
    target_link_libraries(core INTERFACE PyNovaGE::Render)
endif()

# Export and install (temporarily commented due to window GLFW export issue)
# TODO: Re-enable when window export is fixed
# install(TARGETS core
#     EXPORT PyNovaGETargets
# )
