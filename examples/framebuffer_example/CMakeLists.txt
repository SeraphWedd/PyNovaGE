cmake_minimum_required(VERSION 3.20)

set(FRAMEBUFFER_EXAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../framebuffer_example.cpp
)

add_executable(framebuffer_example ${FRAMEBUFFER_EXAMPLE_SOURCES})

target_include_directories(framebuffer_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

target_link_libraries(framebuffer_example PRIVATE
    memory
    math
    window
    renderer
)

set_target_properties(framebuffer_example PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)