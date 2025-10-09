# Foundation components CMake configuration

# Create foundation interface library
add_library(foundation INTERFACE)
add_library(PyNovaGE::Foundation ALIAS foundation)

# Configure math component (only existing one for now)
add_subdirectory(math)

# Link foundation components
target_link_libraries(foundation INTERFACE
    math
)
