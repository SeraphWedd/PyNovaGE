# Benchmark infrastructure module

# Function to setup benchmarks for a component
function(setup_component_benchmarks)
    set(options)
    set(oneValueArgs COMPONENT_NAME BENCH_DIR)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(BENCH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT BENCH_COMPONENT_NAME)
        message(FATAL_ERROR "Component name must be specified for benchmarks")
    endif()

    # Create benchmark executable name
    set(bench_target "${BENCH_COMPONENT_NAME}_benchmarks")

    # Find benchmark sources if not specified
    if(NOT BENCH_SOURCES)
        if(NOT BENCH_BENCH_DIR)
            set(BENCH_BENCH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks")
        endif()

        if(NOT EXISTS "${BENCH_BENCH_DIR}")
            message(STATUS "No benchmarks found for component ${BENCH_COMPONENT_NAME}")
            return()
        endif()

        file(GLOB_RECURSE BENCH_SOURCES
            "${BENCH_BENCH_DIR}/*.cpp"
            "${BENCH_BENCH_DIR}/*.h"
            "${BENCH_BENCH_DIR}/*.hpp"
        )

        if(NOT BENCH_SOURCES)
            message(STATUS "No benchmark sources found for component ${BENCH_COMPONENT_NAME}")
            return()
        endif()
    endif()

    # Create benchmark executable
    add_executable(${bench_target} ${BENCH_SOURCES})

    # Set benchmark properties
    set_target_properties(${bench_target} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        FOLDER "Benchmarks"
    )

    # Add include directories
    target_include_directories(${bench_target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${BENCH_BENCH_DIR}
    )

    # Link with Google Benchmark and component
    target_link_libraries(${bench_target}
        PRIVATE
            ${BENCH_COMPONENT_NAME}
            benchmark::benchmark
            benchmark::benchmark_main
            ${BENCH_DEPENDENCIES}
    )

    # Install benchmarks if requested
    if(PYNOVAGE_INSTALL_BENCHMARKS)
        install(TARGETS ${bench_target}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/benchmarks
            COMPONENT benchmarks
        )
    endif()
endfunction()

# Function to add a standalone benchmark
function(add_benchmark name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(BENCH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Create benchmark executable
    add_executable(${name} ${BENCH_SOURCES})

    # Set benchmark properties
    set_target_properties(${name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        FOLDER "Benchmarks"
    )

    # Link with Google Benchmark and dependencies
    target_link_libraries(${name}
        PRIVATE
            benchmark::benchmark
            benchmark::benchmark_main
            ${BENCH_DEPENDENCIES}
    )
endfunction()

# Function to setup benchmark result analysis
function(setup_benchmark_analysis)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    
    add_custom_target(benchmark_analysis
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/bench_analysis.py
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Analyzing benchmark results"
    )
endfunction()

# Function to run all benchmarks
function(add_benchmark_target)
    # Find all benchmark executables
    file(GLOB_RECURSE benchmark_executables
        "${CMAKE_BINARY_DIR}/bin/*_benchmarks"
        "${CMAKE_BINARY_DIR}/bin/*_benchmarks.exe"
    )

    if(benchmark_executables)
        # Create a target to run all benchmarks
        add_custom_target(run_benchmarks
            COMMAND ${CMAKE_COMMAND} -E echo "Running all benchmarks..."
        )

        foreach(benchmark ${benchmark_executables})
            get_filename_component(benchmark_name ${benchmark} NAME_WE)
            add_custom_command(
                TARGET run_benchmarks
                COMMAND ${benchmark} --benchmark_out=${benchmark_name}.json --benchmark_out_format=json
                COMMAND ${CMAKE_COMMAND} -E echo "Finished ${benchmark_name}"
            )
        endforeach()
    endif()
endfunction()