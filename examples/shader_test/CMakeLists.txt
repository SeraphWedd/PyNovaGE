# Shader System Test - Tests shader compilation and management
add_executable(shader_test main.cpp)

target_link_libraries(shader_test 
    PRIVATE 
    PyNovaGE::Window
    renderer
)

# Set output directory
set_target_properties(shader_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy to examples directory as well for easy access
add_custom_command(TARGET shader_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    $<TARGET_FILE:shader_test> 
    "${CMAKE_BINARY_DIR}/examples/shader_test${CMAKE_EXECUTABLE_SUFFIX}"
    COMMENT "Copying shader_test to examples directory"
)