# Dependencies management module

include(FetchContent)

# Set FetchContent options
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Google Test
function(setup_googletest)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # Prevent overriding parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endfunction()

# Google Benchmark
function(setup_googlebenchmark)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    # Disable tests and install for benchmark
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endfunction()

# Function to setup all dependencies
function(setup_dependencies)
    if(PYNOVAGE_BUILD_TESTS)
        setup_googletest()
    endif()
    
    if(PYNOVAGE_BUILD_BENCHMARKS)
        setup_googlebenchmark()
    endif()
endfunction()

# Helper function to add dependency on Google Test
function(target_link_gtest target)
    if(TARGET gtest AND TARGET gtest_main)
        target_link_libraries(${target}
            PRIVATE
                gtest
                gtest_main
        )
    endif()
endfunction()

# Helper function to add dependency on Google Benchmark
function(target_link_benchmark target)
    if(TARGET benchmark)
        target_link_libraries(${target}
            PRIVATE
                benchmark::benchmark
        )
    endif()
endfunction()