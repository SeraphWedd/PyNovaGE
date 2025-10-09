# Function to add a new engine component with automatic file detection
function(add_engine_component NAME)
    message(STATUS "Configuring component: ${NAME}")

    # Find all source files
    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
    file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
    file(GLOB_RECURSE TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    file(GLOB_RECURSE BENCHMARKS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cpp")

    # Create the library from src/ and include/
    add_library(${NAME} ${SOURCES} ${HEADERS})
    target_include_directories(${NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    # Set up tests if they exist and testing is enabled
    if(TESTS AND PYGE_BUILD_TESTS)
        enable_testing()
        
        # Create test executable
        set(TEST_TARGET "${NAME}_Tests")
        add_executable(${TEST_TARGET} ${TESTS})
        
        # Link against the library and GTest
        target_link_libraries(${TEST_TARGET}
            PRIVATE
                ${NAME}
                GTest::gtest
                GTest::gtest_main
        )
        
        # Add to CTest
        add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
        
        message(STATUS "  Added tests for ${NAME}")
    endif()

    # Set up benchmarks if they exist and benchmarking is enabled
    if(BENCHMARKS AND PYGE_BUILD_BENCHMARKS)
        # Create benchmark executable
        set(BENCH_TARGET "${NAME}_Benchmarks")
        add_executable(${BENCH_TARGET} ${BENCHMARKS})
        
        # Link against the library and Google Benchmark
        target_link_libraries(${BENCH_TARGET}
            PRIVATE
                ${NAME}
                benchmark::benchmark
        )
        
        message(STATUS "  Added benchmarks for ${NAME}")
    endif()

    # Output configuration summary
    message(STATUS "  Sources: ${SOURCES}")
    message(STATUS "  Headers: ${HEADERS}")
    if(TESTS)
        message(STATUS "  Tests: ${TESTS}")
    endif()
    if(BENCHMARKS)
        message(STATUS "  Benchmarks: ${BENCHMARKS}")
    endif()
endfunction()