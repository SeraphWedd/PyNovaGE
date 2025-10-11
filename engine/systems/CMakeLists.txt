# Engine systems CMake configuration

# Add asset system
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/asset/CMakeLists.txt")
    add_subdirectory(asset)
endif()

# Future systems can be added here
# add_subdirectory(audio)
# add_subdirectory(scene)
# add_subdirectory(input)

message(STATUS "Systems directory configured")