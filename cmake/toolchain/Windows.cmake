# Windows toolchain configuration

include(${CMAKE_CURRENT_LIST_DIR}/Base.cmake)

# Windows-specific settings
set(CMAKE_SYSTEM_NAME Windows)

# Set Windows SDK version if not specified
if(NOT CMAKE_SYSTEM_VERSION)
    set(CMAKE_SYSTEM_VERSION 10.0)
endif()

# MSVC-specific settings
if(MSVC)
    # Enable multi-processor compilation
    add_compile_options(/MP)
    
    # Use static runtime by default
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Windows-specific compile definitions
    add_compile_definitions(
        _WINDOWS
        UNICODE
        _UNICODE
        NOMINMAX  # Disable min/max macros from windows.h
        WIN32_LEAN_AND_MEAN
    )
    
    # Enable source file debugging
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# DirectX support
find_package(D3D12 QUIET)
if(D3D12_FOUND)
    add_compile_definitions(PYNOVAGE_ENABLE_DX12)
endif()

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