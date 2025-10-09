# Component management module

# Function to add a new engine component
function(add_engine_component name)
    set(options)
    set(oneValueArgs TYPE)
    set(multiValueArgs SOURCES HEADERS PUBLIC_HEADERS DEPENDENCIES)
    cmake_parse_arguments(COMPONENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate component type
    if(NOT COMPONENT_TYPE)
        message(FATAL_ERROR "Component type must be specified for ${name}")
    endif()

    # Create the component library
    add_library(${name} ${COMPONENT_SOURCES} ${COMPONENT_HEADERS})
    add_library(PyNovaGE::${name} ALIAS ${name})

    # Set include directories
    target_include_directories(${name}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Link dependencies
    if(COMPONENT_DEPENDENCIES)
        target_link_libraries(${name}
            PUBLIC
                ${COMPONENT_DEPENDENCIES}
        )
    endif()

    # Set properties
    set_target_properties(${name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
    )

    # Add tests if enabled
    if(PYNOVAGE_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
        add_subdirectory(tests)
    endif()

    # Add benchmarks if enabled
    if(PYNOVAGE_BUILD_BENCHMARKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks")
        add_subdirectory(benchmarks)
    endif()
endfunction()

# Function to add component tests
function(add_component_test name component)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Create the test executable
    add_executable(${name} ${TEST_SOURCES})

    # Link with GTest and the component
    target_link_libraries(${name}
        PRIVATE
            ${component}
            gtest
            gtest_main
            ${TEST_DEPENDENCIES}
    )

    # Add the test
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Function to add component benchmark
function(add_component_benchmark name component)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(BENCH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Create the benchmark executable
    add_executable(${name} ${BENCH_SOURCES})

    # Link with Google Benchmark and the component
    target_link_libraries(${name}
        PRIVATE
            ${component}
            benchmark::benchmark
            ${BENCH_DEPENDENCIES}
    )
endfunction()

# Function to automatically find and add source files
function(auto_add_sources target)
    set(options)
    set(oneValueArgs SOURCE_DIR HEADER_DIR)
    set(multiValueArgs PATTERNS)
    cmake_parse_arguments(AUTO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Default patterns if none specified
    if(NOT AUTO_PATTERNS)
        set(AUTO_PATTERNS "*.cpp" "*.hpp" "*.h")
    endif()

    # Find all source files
    foreach(pattern ${AUTO_PATTERNS})
        file(GLOB_RECURSE sources
            "${AUTO_SOURCE_DIR}/${pattern}"
            "${AUTO_HEADER_DIR}/${pattern}"
        )
        list(APPEND all_sources ${sources})
    endforeach()

    # Add sources to the target
    target_sources(${target}
        PRIVATE
            ${all_sources}
    )
endfunction()