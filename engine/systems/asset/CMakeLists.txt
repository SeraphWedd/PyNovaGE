cmake_minimum_required(VERSION 3.15)
project(asset_system)

# Find source files
file(GLOB_RECURSE ASSET_SOURCES "src/*.cpp")
file(GLOB_RECURSE ASSET_HEADERS "include/*.hpp")

# Create library
add_library(asset STATIC ${ASSET_SOURCES} ${ASSET_HEADERS})

# Include directories
target_include_directories(asset PUBLIC 
    include
    ${CMAKE_SOURCE_DIR}/engine/foundation/math/include
    ${CMAKE_SOURCE_DIR}/engine/core/renderer/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# Link dependencies
target_link_libraries(asset 
    math
    renderer
)

# Add STB libraries
add_library(stb_libs STATIC
    ${CMAKE_SOURCE_DIR}/third_party/stb/stb_image.cpp
    ${CMAKE_SOURCE_DIR}/third_party/stb/stb_image_write.cpp  
    ${CMAKE_SOURCE_DIR}/third_party/stb/stb_truetype.cpp
)

target_include_directories(stb_libs PUBLIC ${CMAKE_SOURCE_DIR}/third_party)
target_link_libraries(asset stb_libs)

# Compiler flags
if(MSVC)
    target_compile_options(asset PRIVATE /W4 /WX-)
else()
    target_compile_options(asset PRIVATE -Wall -Wextra)
endif()

# Add tests if enabled
if(PYNOVAGE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

message(STATUS "Asset system configured with ${CMAKE_CURRENT_LIST_DIR}")