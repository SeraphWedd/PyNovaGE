# Linux toolchain configuration

include(${CMAKE_CURRENT_LIST_DIR}/Base.cmake)

# Linux-specific settings
set(CMAKE_SYSTEM_NAME Linux)

# Linux-specific compile definitions
add_compile_definitions(
    __linux__
    LINUX
)

# Linux-specific compiler flags
add_compile_options(
    -fPIC
    -pthread
)

# Add linker flags
add_link_options(-pthread)

# Vulkan support
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    add_compile_definitions(PYNOVAGE_ENABLE_VULKAN)
endif()

# OpenGL support
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    add_compile_definitions(PYNOVAGE_ENABLE_OPENGL)
endif()

# X11 support
find_package(X11 QUIET)
if(X11_FOUND)
    add_compile_definitions(PYNOVAGE_ENABLE_X11)
endif()

# Wayland support
find_package(ECM QUIET NO_MODULE)
if(ECM_FOUND)
    list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
    find_package(Wayland QUIET)
    if(Wayland_FOUND)
        add_compile_definitions(PYNOVAGE_ENABLE_WAYLAND)
    endif()
endif()

# Check for various system libraries
find_library(DL_LIB dl)
find_library(M_LIB m)
find_library(RT_LIB rt)