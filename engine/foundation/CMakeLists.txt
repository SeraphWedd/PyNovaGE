# Foundation components CMake configuration

# Create foundation interface library
add_library(foundation INTERFACE)
add_library(PyNovaGE::Foundation ALIAS foundation)

# Configure foundation components
add_subdirectory(math)
add_subdirectory(memory)
add_subdirectory(threading)

# Link foundation components
target_link_libraries(foundation INTERFACE
    math
    memory
    threading
)
