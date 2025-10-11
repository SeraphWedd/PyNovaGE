# Audio System with OpenAL integration
cmake_minimum_required(VERSION 3.20)

# Configure Audio System
message(STATUS "Configuring Audio System...")

# Find OpenAL
find_package(OpenAL)
if(NOT OpenAL_FOUND)
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
        ${OpenAL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(audio
    PUBLIC
        ${OpenAL_LIBRARIES}
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