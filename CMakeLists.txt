cmake_minimum_required(VERSION 3.20)

# Set policies
cmake_policy(SET CMP0048 NEW)  # project() command manages VERSION variables
cmake_policy(SET CMP0077 NEW)  # option() honors normal variables

# Project declaration
project(PyNovaGE
    VERSION 0.1.0
    DESCRIPTION "Python Game Engine with modern C++ core"
    HOMEPAGE_URL "https://github.com/yourusername/PyNovaGE"
    LANGUAGES CXX
)

# Add CMake modules path
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config"
)

# Include core modules
include(Platform)
include(CompilerConfig)
include(Dependencies)
include(Component)
include(Testing)
include(Benchmarking)
include(EngineComponent)

# Build options
option(PYNOVAGE_BUILD_SHARED "Build engine as shared libraries" OFF)
option(PYNOVAGE_BUILD_TESTS "Build test suite" ON)
option(PYNOVAGE_BUILD_BENCHMARKS "Build benchmarks" ON)
option(PYNOVAGE_BUILD_EXAMPLES "Build example applications" OFF)
option(PYNOVAGE_BUILD_TOOLS "Build development tools" OFF)
option(PYNOVAGE_BUILD_DOCS "Build documentation" OFF)
option(PYNOVAGE_ENABLE_LTO "Enable Link Time Optimization" OFF)
option(PYNOVAGE_ENABLE_UNITY "Enable Unity builds" OFF)
option(PYNOVAGE_INSTALL_TESTS "Install test executables" OFF)
option(PYNOVAGE_INSTALL_BENCHMARKS "Install benchmark executables" OFF)

# Configure version header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/pynovage/Version.h
)

# Setup external dependencies
setup_dependencies()

# Engine components
add_subdirectory(engine)

# Tools
if(PYNOVAGE_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# Examples
if(PYNOVAGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Enable testing if requested
if(PYNOVAGE_BUILD_TESTS)
    enable_testing()
endif()
# Documentation
if(PYNOVAGE_BUILD_DOCS)
    add_subdirectory(docs)
endif()

# Export targets
include(CMakePackageConfigHelpers)

# Generate the config file that includes the exports
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PyNovaGEConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PyNovaGE
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Generate version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/PyNovaGEConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/PyNovaGEConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PyNovaGEConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PyNovaGE
)

# Install targets configuration
install(EXPORT PyNovaGETargets
    FILE PyNovaGETargets.cmake
    NAMESPACE PyNovaGE::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PyNovaGE
)

# Install headers
install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/pynovage
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Print configuration summary
include(FeatureSummary)
feature_summary(WHAT ALL)