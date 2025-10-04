add_library(memory INTERFACE)

target_include_directories(memory
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# Benchmarks
if(PYNOVAGE_BUILD_BENCHMARKS)
    # Linear allocator benchmarks
    add_executable(allocator_benchmarks_linear
        tests/allocator_perf_tests.cpp
        tests/memory_benchmarks.cpp
    )
    target_link_libraries(allocator_benchmarks_linear
        PRIVATE
            memory
            benchmark::benchmark_main
    )

    # Pool allocator benchmarks
    add_executable(allocator_benchmarks_pool
        tests/pool_allocator_perf_tests.cpp
        tests/memory_benchmarks.cpp
    )
    target_link_libraries(allocator_benchmarks_pool
        PRIVATE
            memory
            benchmark::benchmark_main
    )

    # Stack allocator benchmarks
    add_executable(allocator_benchmarks_stack
        tests/stack_allocator_perf_tests.cpp
        tests/memory_benchmarks.cpp
    )
    target_link_libraries(allocator_benchmarks_stack
        PRIVATE
            memory
            benchmark::benchmark_main
    )

    # Defragmenting allocator benchmarks - split into separate executables
    add_executable(defrag_basic_benchmarks
        tests/basic_ops_main.cpp
    )
    target_link_libraries(defrag_basic_benchmarks
        PRIVATE
            memory
            benchmark::benchmark_main
    )
    
    add_executable(defrag_threading_benchmarks
        tests/threading_main.cpp
    )
    target_link_libraries(defrag_threading_benchmarks
        PRIVATE
            memory
            benchmark::benchmark_main
    )
    
    add_executable(defrag_sizeclass_benchmarks
        tests/sizeclass_main.cpp
    )
    target_link_libraries(defrag_sizeclass_benchmarks
        PRIVATE
            memory
            benchmark::benchmark_main
    )
endif()

# Vector tests
add_executable(vector_tests
    tests/vector_tests.cpp
)
target_link_libraries(vector_tests
    PRIVATE
        memory
        GTest::gtest_main
        GTest::gmock
)
add_test(NAME vector_tests COMMAND vector_tests)

# Defrag allocator tests
add_executable(defrag_allocator_tests
    tests/defrag_allocator_tests.cpp
)
target_link_libraries(defrag_allocator_tests
    PRIVATE
        memory
        GTest::gtest_main
        GTest::gmock
)
add_test(NAME defrag_allocator_tests COMMAND defrag_allocator_tests)

# String tests
add_executable(string_tests
    tests/string_tests.cpp
)
target_link_libraries(string_tests
    PRIVATE
        memory
        GTest::gtest_main
        GTest::gmock
)
add_test(NAME string_tests COMMAND string_tests)

# String view tests
add_executable(string_view_tests
    tests/string_view_tests.cpp
)
target_link_libraries(string_view_tests
    PRIVATE
        memory
        GTest::gtest_main
        GTest::gmock
)
add_test(NAME string_view_tests COMMAND string_view_tests)

# UTF tests
add_executable(utf_tests
    tests/utf_tests.cpp
)
target_link_libraries(utf_tests
    PRIVATE
        memory
        GTest::gtest_main
        GTest::gmock
)
add_test(NAME utf_tests COMMAND utf_tests)

# Memory analysis benchmarks
if(PYNOVAGE_BUILD_BENCHMARKS)
    add_executable(memory_analysis_benchmarks
        tests/memory_analysis_benchmarks.cpp
    )
    target_link_libraries(memory_analysis_benchmarks
        PRIVATE
            memory
            benchmark::benchmark_main
    )
endif()

add_library(nova_memory INTERFACE)
