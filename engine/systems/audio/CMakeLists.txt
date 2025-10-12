# Audio System with OpenAL integration
cmake_minimum_required(VERSION 3.20)

# Configure Audio System
message(STATUS "Configuring Audio System...")

# Find OpenAL - first try system, then third_party
set(OPENAL_FOUND FALSE)
set(OPENAL_INCLUDE_DIRS "")
set(OPENAL_LIBRARIES "")

find_package(OpenAL)
if(OpenAL_FOUND)
    set(OPENAL_FOUND TRUE)
    set(OPENAL_INCLUDE_DIRS ${OpenAL_INCLUDE_DIRS})
    set(OPENAL_LIBRARIES ${OpenAL_LIBRARIES})
    message(STATUS "Found system OpenAL: ${OPENAL_LIBRARIES}")
else()
    # Try third_party OpenAL
    set(OPENAL_ROOT "${CMAKE_SOURCE_DIR}/third_party/openal")
    if(EXISTS "${OPENAL_ROOT}/include/AL/al.h")
        set(OPENAL_FOUND TRUE)
        set(OPENAL_INCLUDE_DIRS "${OPENAL_ROOT}/include")
        
        # Determine architecture and select appropriate library
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            # 64-bit
            set(OPENAL_LIB_DIR "${OPENAL_ROOT}/libs/Win64")
            set(OPENAL_DLL_DIR "${OPENAL_ROOT}/bin/Win64")
        else()
            # 32-bit
            set(OPENAL_LIB_DIR "${OPENAL_ROOT}/libs/Win32")
            set(OPENAL_DLL_DIR "${OPENAL_ROOT}/bin/Win32")
        endif()
        
        if(EXISTS "${OPENAL_LIB_DIR}/OpenAL32.lib")
            set(OPENAL_LIBRARIES "${OPENAL_LIB_DIR}/OpenAL32.lib")
            message(STATUS "Found third_party OpenAL: ${OPENAL_LIBRARIES}")
        else()
            set(OPENAL_FOUND FALSE)
        endif()
    endif()
endif()

if(NOT OPENAL_FOUND)
    message(WARNING "OpenAL not found! Audio system will be skipped. Install OpenAL development libraries to enable audio.")
    return()
endif()

# Collect source files automatically
file(GLOB_RECURSE AUDIO_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE AUDIO_HEADERS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# Create audio library
add_library(audio STATIC ${AUDIO_SOURCES} ${AUDIO_HEADERS})

# Set target properties
set_target_properties(audio PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER "Engine/Systems"
)

# Include directories
target_include_directories(audio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${OPENAL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(audio
    PUBLIC
        ${OPENAL_LIBRARIES}
        math  # For 3D position calculations
    PRIVATE
        asset  # For AudioClip integration
)

# Platform-specific configuration
if(WIN32)
    # Windows-specific OpenAL configuration
    target_compile_definitions(audio PRIVATE AL_LIBTYPE_STATIC)
endif()

# Configure tests if enabled
if(PYNOVAGE_BUILD_TESTS)
    message(STATUS "Configuring Audio System tests...")
    add_subdirectory(tests)
endif()

# Summary
message(STATUS "Audio System configured:")
message(STATUS "  - ${AUDIO_SOURCES}")
message(STATUS "  - ${AUDIO_HEADERS}")
message(STATUS "  - OpenAL integration: OK")
message(STATUS "  - Tests: ${PYNOVAGE_BUILD_TESTS}")
message(STATUS "Audio System configuration complete.")