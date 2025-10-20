# MMO Performance Demo Example
add_executable(mmo_performance_demo main.cpp)

# Link against required libraries
target_link_libraries(mmo_performance_demo PRIVATE 
    renderer
    window
    scene  
    memory
    threading
    math
    glfw
    ${OPENGL_LIBRARIES}
)

# Enable C++17
target_compile_features(mmo_performance_demo PRIVATE cxx_std_17)

# Enable maximum optimizations for performance demo
if(MSVC)
    target_compile_options(mmo_performance_demo PRIVATE $<$<CONFIG:Release>:/O2 /Ob3 /GL>)
    target_link_options(mmo_performance_demo PRIVATE $<$<CONFIG:Release>:/LTCG>)
else()
    target_compile_options(mmo_performance_demo PRIVATE $<$<CONFIG:Release>:-O3 -march=native -flto>)
    target_link_options(mmo_performance_demo PRIVATE $<$<CONFIG:Release>:-flto>)
endif()

# Set output directory
set_target_properties(mmo_performance_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy to examples directory for easy access
add_custom_command(TARGET mmo_performance_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "$<TARGET_FILE:mmo_performance_demo>"
    "${CMAKE_BINARY_DIR}/examples/mmo_performance_demo.exe"
    COMMENT "Copying MMO Performance Demo to examples directory"
)
