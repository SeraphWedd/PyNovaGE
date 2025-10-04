cmake_minimum_required(VERSION 3.15)
project(PyNovaGE VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Build options
option(PYNOVAGE_BUILD_BENCHMARKS "Build benchmark executables" ON)
option(PROFILE_BROADPHASE "Enable detailed profiling of broad phase" OFF)

# Add subdirectories
add_subdirectory(core)
