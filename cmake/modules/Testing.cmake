# Test infrastructure module

include(CTest)
include(GoogleTest)

# Function to setup tests for a component
function(setup_component_tests)
    set(options)
    set(oneValueArgs COMPONENT_NAME TEST_DIR)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT TEST_COMPONENT_NAME)
        message(FATAL_ERROR "Component name must be specified for tests")
    endif()

    # Create test executable name
    set(test_target "${TEST_COMPONENT_NAME}_tests")

    # Find test sources if not specified
    if(NOT TEST_SOURCES)
        if(NOT TEST_TEST_DIR)
            set(TEST_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
        endif()

        if(NOT EXISTS "${TEST_TEST_DIR}")
            message(STATUS "No tests found for component ${TEST_COMPONENT_NAME}")
            return()
        endif()

        file(GLOB_RECURSE TEST_SOURCES
            "${TEST_TEST_DIR}/*.cpp"
            "${TEST_TEST_DIR}/*.h"
            "${TEST_TEST_DIR}/*.hpp"
        )

        if(NOT TEST_SOURCES)
            message(STATUS "No test sources found for component ${TEST_COMPONENT_NAME}")
            return()
        endif()
    endif()

    # Create test executable
    add_executable(${test_target} ${TEST_SOURCES})

    # Set test properties
    set_target_properties(${test_target} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        FOLDER "Tests"
    )

    # Add include directories
    target_include_directories(${test_target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${TEST_TEST_DIR}
    )

    # Link with GTest and component
    target_link_libraries(${test_target}
        PRIVATE
            ${TEST_COMPONENT_NAME}
            GTest::gtest
            GTest::gtest_main
            ${TEST_DEPENDENCIES}
    )

    # Add test to CTest
    gtest_discover_tests(${test_target}
        PROPERTIES
            LABELS "${TEST_COMPONENT_NAME}"
        DISCOVERY_TIMEOUT 60
    )

    # Install tests if requested
    if(PYNOVAGE_INSTALL_TESTS)
        install(TARGETS ${test_target}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
            COMPONENT tests
        )
    endif()
endfunction()

# Function to setup a test suite
function(add_test_suite name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(SUITE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Create test suite executable
    add_executable(${name} ${SUITE_SOURCES})

    # Set test properties
    set_target_properties(${name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        FOLDER "Tests"
    )

    # Link with GTest and dependencies
    target_link_libraries(${name}
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            ${SUITE_DEPENDENCIES}
    )

    # Add test to CTest
    gtest_discover_tests(${name}
        PROPERTIES
            LABELS "${name}"
        DISCOVERY_TIMEOUT 60
    )
endfunction()

# Function to generate test coverage report
function(setup_test_coverage)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Check if gcov is installed
        find_program(GCOV_PATH gcov)
        if(NOT GCOV_PATH)
            message(WARNING "gcov not found, coverage report generation disabled")
            return()
        endif()

        # Add coverage flags
        add_compile_options(--coverage)
        add_link_options(--coverage)

        # Find lcov
        find_program(LCOV_PATH lcov)
        if(LCOV_PATH)
            add_custom_target(coverage
                COMMAND ${LCOV_PATH} --directory . --zerocounters
                COMMAND ctest
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info.cleaned
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report"
            )
        endif()
    endif()
endfunction()