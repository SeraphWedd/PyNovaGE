# Particle System Module
cmake_minimum_required(VERSION 3.16)

# Add particle system library
set(PARTICLE_SYSTEM_SOURCES
    src/particle.cpp
    src/particle_emitter.cpp
    src/particle_system.cpp
)

set(PARTICLE_SYSTEM_HEADERS
    include/particles/particle.hpp
    include/particles/particle_emitter.hpp
    include/particles/particle_system.hpp
)

# Create particle system library
add_library(particle_system ${PARTICLE_SYSTEM_SOURCES} ${PARTICLE_SYSTEM_HEADERS})

# Set include directories
target_include_directories(particle_system
    PUBLIC 
        include
    PRIVATE
        src
)

# Link required dependencies
target_link_libraries(particle_system
    PUBLIC
        math
        memory
        renderer
    PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
)

# Set C++ standard
target_compile_features(particle_system PUBLIC cxx_std_17)

# Add compiler-specific options
if(MSVC)
    target_compile_options(particle_system PRIVATE /W4)
else()
    target_compile_options(particle_system PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ========== Tests ==========
if(BUILD_TESTS)
    # Particle system tests
    add_executable(particle_system_tests
        tests/test_particle_system.cpp
    )
    
    target_link_libraries(particle_system_tests
        PRIVATE
            particle_system
            gtest
            gtest_main
    )
    
    # Add to test suite
    add_test(NAME ParticleSystemTests COMMAND particle_system_tests)
    
    # Set working directory for tests
    set_target_properties(particle_system_tests PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# ========== Benchmarks ==========
if(BUILD_BENCHMARKS)
    # Find Google Benchmark
    find_package(benchmark QUIET)
    
    if(benchmark_FOUND)
        add_executable(particle_system_benchmarks
            benchmarks/benchmark_particle_system.cpp
        )
        
        target_link_libraries(particle_system_benchmarks
            PRIVATE
                particle_system
                benchmark::benchmark
        )
        
        # Set optimization for benchmarks
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            target_compile_options(particle_system_benchmarks PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:/O2>
                $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3>
            )
        endif()
        
        set_target_properties(particle_system_benchmarks PROPERTIES
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    else()
        message(WARNING "Google Benchmark not found. Particle system benchmarks will not be built.")
    endif()
endif()

# ========== Installation ==========
if(BUILD_SHARED_LIBS)
    install(TARGETS particle_system
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
    
    install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
    )
endif()

# ========== Documentation ==========
if(BUILD_DOCUMENTATION)
    # Add particle system to Doxygen documentation
    set(DOXYGEN_PARTICLE_SYSTEM_SOURCES ${PARTICLE_SYSTEM_HEADERS})
    list(APPEND DOXYGEN_INPUT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()