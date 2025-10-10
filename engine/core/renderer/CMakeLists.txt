# Renderer Module
cmake_minimum_required(VERSION 3.20)

# Find OpenGL
find_package(OpenGL REQUIRED)

# GLAD OpenGL loader
set(GLAD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/glad/src/gl.c
)

# Renderer library
add_library(renderer STATIC
    # Core renderer
    src/renderer.cpp
    src/shader.cpp
    src/texture.cpp
    src/sprite_renderer.cpp
    src/batch_renderer.cpp
    
    # OpenGL utilities
    src/gl_state.cpp
    src/gl_buffer.cpp
    
    # GLAD loader (if available)
    ${GLAD_SOURCES}
)

target_include_directories(renderer PUBLIC
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/glad/include
)

target_link_libraries(renderer PUBLIC
    math
    memory
    window
    ${OPENGL_LIBRARIES}
)

# Platform-specific OpenGL linking
if(WIN32)
    target_link_libraries(renderer PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(renderer PRIVATE "-framework OpenGL")
else()
    target_link_libraries(renderer PRIVATE GL)
endif()

# Compiler-specific settings
if(MSVC)
    target_compile_options(renderer PRIVATE /W4)
else()
    target_compile_options(renderer PRIVATE -Wall -Wextra -pedantic)
endif()

# Tests
if(BUILD_TESTS)
    add_executable(renderer_tests
        tests/test_renderer.cpp
        tests/test_shader.cpp
        tests/test_texture.cpp
        tests/test_texture_atlas.cpp
        tests/test_sprite_renderer.cpp
        tests/test_batch_renderer.cpp
    )

    target_link_libraries(renderer_tests PRIVATE
        renderer
        gtest_main
    )
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    add_executable(renderer_benchmarks
        benchmarks/renderer_bench.cpp
    )

    target_link_libraries(renderer_benchmarks PRIVATE
        renderer
        benchmark_main
    )
endif()