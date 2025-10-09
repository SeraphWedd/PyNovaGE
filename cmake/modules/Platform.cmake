# Platform detection and configuration module

# Detect platform
if(WIN32)
    set(PYNOVAGE_PLATFORM "windows" CACHE STRING "Target platform")
elseif(ANDROID)
    set(PYNOVAGE_PLATFORM "android" CACHE STRING "Target platform")
elseif(UNIX AND NOT APPLE)
    set(PYNOVAGE_PLATFORM "linux" CACHE STRING "Target platform")
elseif(APPLE)
    set(PYNOVAGE_PLATFORM "macos" CACHE STRING "Target platform")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Platform-specific settings
set(PYNOVAGE_PLATFORM_WINDOWS "windows")
set(PYNOVAGE_PLATFORM_LINUX "linux")
set(PYNOVAGE_PLATFORM_ANDROID "android")
set(PYNOVAGE_PLATFORM_MACOS "macos")

# Architecture detection
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PYNOVAGE_ARCH "x64")
else()
    set(PYNOVAGE_ARCH "x86")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64.*|AARCH64.*|arm64.*|ARM64.*)")
    set(PYNOVAGE_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
    set(PYNOVAGE_ARCH "arm")
endif()

# Platform-specific compile definitions
if(WIN32)
    add_compile_definitions(PYNOVAGE_PLATFORM_WINDOWS)
elseif(ANDROID)
    add_compile_definitions(PYNOVAGE_PLATFORM_ANDROID)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(PYNOVAGE_PLATFORM_LINUX)
elseif(APPLE)
    add_compile_definitions(PYNOVAGE_PLATFORM_MACOS)
endif()

# Export platform information
set(PYNOVAGE_PLATFORM_TRIPLE "${PYNOVAGE_PLATFORM}-${PYNOVAGE_ARCH}")
message(STATUS "Target platform: ${PYNOVAGE_PLATFORM_TRIPLE}")