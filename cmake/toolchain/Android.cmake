# Android toolchain configuration

include(${CMAKE_CURRENT_LIST_DIR}/Base.cmake)

# Android-specific settings
set(CMAKE_SYSTEM_NAME Android)

# Set default Android API level if not specified
if(NOT CMAKE_SYSTEM_VERSION)
    set(CMAKE_SYSTEM_VERSION 29)  # Android 10.0
endif()

# Set default Android ABI if not specified
if(NOT CMAKE_ANDROID_ARCH_ABI)
    set(CMAKE_ANDROID_ARCH_ABI "arm64-v8a")
endif()

# Set default STL type
set(CMAKE_ANDROID_STL_TYPE c++_static)

# Android-specific compile definitions
add_compile_definitions(
    __ANDROID__
    ANDROID
)

# Android-specific compiler flags
add_compile_options(
    -fPIC
    -fstack-protector-strong
    -fno-strict-aliasing
)

# Find Android NDK
if(NOT ANDROID_NDK)
    if(DEFINED ENV{ANDROID_NDK_ROOT})
        set(ANDROID_NDK $ENV{ANDROID_NDK_ROOT})
    elseif(DEFINED ENV{ANDROID_NDK_HOME})
        set(ANDROID_NDK $ENV{ANDROID_NDK_HOME})
    else()
        message(FATAL_ERROR "Android NDK not found. Please set ANDROID_NDK_ROOT or ANDROID_NDK_HOME environment variable.")
    endif()
endif()

# Vulkan support for Android
find_library(VULKAN_LIB vulkan)
if(VULKAN_LIB)
    add_compile_definitions(PYNOVAGE_ENABLE_VULKAN)
endif()

# OpenGL ES support
find_library(GLES3_LIB GLESv3)
if(GLES3_LIB)
    add_compile_definitions(PYNOVAGE_ENABLE_GLES3)
endif()

# Android-specific libraries
find_library(ANDROID_LIB android)
find_library(LOG_LIB log)
find_library(NATIVE_APP_GLUE_LIB native_app_glue)

# Set Android game activity library
set(GAME_ACTIVITY_LIB_PATH "${ANDROID_NDK}/sources/android/game-activity")
if(EXISTS "${GAME_ACTIVITY_LIB_PATH}")
    add_compile_definitions(PYNOVAGE_USE_GAME_ACTIVITY)
    include_directories("${GAME_ACTIVITY_LIB_PATH}/include")
endif()