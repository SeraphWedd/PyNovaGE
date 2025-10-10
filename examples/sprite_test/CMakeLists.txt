# Sprite Rendering Test Example
add_executable(sprite_test main.cpp)

# Link against required libraries
target_link_libraries(sprite_test PRIVATE 
    renderer
    window  
    memory
    glfw
    ${OPENGL_LIBRARIES}
)

# Set output directory
set_target_properties(sprite_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy to examples directory for easy access
add_custom_command(TARGET sprite_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "$<TARGET_FILE:sprite_test>"
    "${CMAKE_BINARY_DIR}/examples/sprite_test.exe"
    COMMENT "Copying sprite_test to examples directory"
)