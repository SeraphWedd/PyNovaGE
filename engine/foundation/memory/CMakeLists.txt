# Memory Management Module
set(MEMORY_SOURCES
    include/memory/allocator.h
    include/memory/memory_pool.h
    include/memory/stack_allocator.h
    include/memory/object_pool.h
    src/allocator.cpp
    src/memory_pool.cpp
    src/stack_allocator.cpp
)

set(MEMORY_HEADERS
    include/memory/allocator.h
    include/memory/memory_pool.h
    include/memory/stack_allocator.h
    include/memory/object_pool.h
)

# Create memory static library
add_library(memory STATIC ${MEMORY_SOURCES})
target_include_directories(memory PUBLIC include)
target_compile_features(memory PUBLIC cxx_std_20)

# Enable SIMD optimizations
if(MSVC)
    target_compile_options(memory PRIVATE /arch:AVX2)
else()
    target_compile_options(memory PRIVATE -mavx2 -mfma)
endif()

# Tests
if(PYNOVAGE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Benchmarks
if(PYNOVAGE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
