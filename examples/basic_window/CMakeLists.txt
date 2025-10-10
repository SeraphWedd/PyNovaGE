cmake_minimum_required(VERSION 3.20)

# Basic Window Demo
add_executable(basic_window_demo
    main.cpp
)

# Link with our engine libraries
target_link_libraries(basic_window_demo PRIVATE
    window
    renderer
    math
    memory
)

# Include directories  
target_include_directories(basic_window_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/core/window/include
    ${CMAKE_SOURCE_DIR}/engine/core/renderer/include
    ${CMAKE_SOURCE_DIR}/engine/foundation/math/include
    ${CMAKE_SOURCE_DIR}/engine/foundation/memory/include
)

# Set output directory
set_target_properties(basic_window_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/basic_window
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings if needed
    target_compile_definitions(basic_window_demo PRIVATE 
        PYNOVAGE_PLATFORM_WINDOWS
    )
elseif(APPLE)
    # macOS-specific settings
    target_compile_definitions(basic_window_demo PRIVATE 
        PYNOVAGE_PLATFORM_MACOS
    )
else()
    # Linux-specific settings  
    target_compile_definitions(basic_window_demo PRIVATE 
        PYNOVAGE_PLATFORM_LINUX
    )
endif()

# Compiler-specific settings
if(MSVC)
    target_compile_options(basic_window_demo PRIVATE /W4)
else()
    target_compile_options(basic_window_demo PRIVATE -Wall -Wextra -pedantic)
endif()