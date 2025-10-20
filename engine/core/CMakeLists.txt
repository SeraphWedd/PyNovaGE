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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/renderer/CMakeLists.txt")
    add_subdirectory(renderer)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/scene/CMakeLists.txt")
    add_subdirectory(scene)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/camera/CMakeLists.txt")
    add_subdirectory(camera)
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

if(TARGET renderer)
    target_link_libraries(core INTERFACE renderer)
endif()

if(TARGET scene)
    target_link_libraries(core INTERFACE PyNovaGE::Scene)
endif()

if(TARGET camera)
    target_link_libraries(core INTERFACE PyNovaGE::Camera)
endif()

# Export and install (temporarily commented due to window GLFW export issue)
# TODO: Re-enable when window export is fixed
# install(TARGETS core
#     EXPORT PyNovaGETargets
# )
