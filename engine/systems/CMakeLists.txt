# Engine systems CMake configuration

# Add asset system
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/asset/CMakeLists.txt")
    add_subdirectory(asset)
endif()

# Add audio system
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/audio/CMakeLists.txt")
    add_subdirectory(audio)
endif()

# Future systems can be added here
# add_subdirectory(scene)
# add_subdirectory(input)

message(STATUS "Systems directory configured")