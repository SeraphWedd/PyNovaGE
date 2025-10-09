# Memory Benchmarks

set(MEMORY_BENCHMARK_SOURCES
    memory_allocator_bench.cpp
)

add_executable(memory_allocator_bench ${MEMORY_BENCHMARK_SOURCES})

target_link_libraries(memory_allocator_bench
    memory
    benchmark::benchmark
    benchmark::benchmark_main
)

target_include_directories(memory_allocator_bench PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Set benchmark properties
set_target_properties(memory_allocator_bench PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    FOLDER "Benchmarks"
)
