# Interactive Demo - Tests both window and input systems
add_executable(interactive_demo main.cpp)

target_link_libraries(interactive_demo 
    PRIVATE 
    PyNovaGE::Window
    renderer
)

# Set output directory
set_target_properties(interactive_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy to examples directory as well for easy access
add_custom_command(TARGET interactive_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    $<TARGET_FILE:interactive_demo> 
    "${CMAKE_BINARY_DIR}/examples/interactive_demo${CMAKE_EXECUTABLE_SUFFIX}"
    COMMENT "Copying interactive_demo to examples directory"
)