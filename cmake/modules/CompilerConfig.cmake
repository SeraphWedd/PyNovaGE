# Compiler configuration and optimization settings

include(CheckIPOSupported)
include(CheckCXXCompilerFlag)

# Detect compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(PYNOVAGE_COMPILER_MSVC TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(PYNOVAGE_COMPILER_CLANG TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(PYNOVAGE_COMPILER_GCC TRUE)
endif()

# Set warning levels
if(PYNOVAGE_COMPILER_MSVC)
    add_compile_options(/W4 /WX)
    # Disable specific warnings
    add_compile_options(/wd4251) # class needs to have dll-interface
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Enable SIMD instructions based on platform
if(PYNOVAGE_COMPILER_MSVC)
    # Enable all instruction sets
    add_compile_options(/arch:AVX2)    # AVX2, AVX, SSE4.2, SSE4.1, SSE3, SSE2
    
    # Enable enhanced instruction sets and optimizations
    add_compile_options(/fp:fast)       # Fast floating point model always
    add_compile_options(/GL)            # Whole program optimization
    add_compile_options(/Oi)            # Enable intrinsic functions
    add_compile_options(/Ot)            # Favor fast code
    add_compile_options(/Qpar)          # Auto-Parallelizer
    add_compile_options(/Qvec-report:2) # Show auto-vectorization report
    add_compile_options(/GT)            # Fiber-safe optimizations
    add_compile_options(/GS-)           # Disable buffer security check for performance
    add_compile_options(/Zo)            # Enhanced debugging info
    
    # Additional Release optimizations
    add_compile_options($<$<CONFIG:Release>:/O2>)        # Maximum optimization
    add_compile_options($<$<CONFIG:Release>:/Ob3>)      # Most aggressive inlining
    add_compile_options($<$<CONFIG:Release>:/Oy>)       # Frame pointer omission
    add_compile_options($<$<CONFIG:Release>:/favor:INTEL64>) # Optimize for modern Intel
    
    # Memory alignment for SIMD
    add_compile_definitions(ALIGN_MALLOC)
    add_compile_definitions(_ENABLE_EXTENDED_ALIGNED_STORAGE)
    
    # Enable enhanced CPU features
    add_compile_definitions(__FMA__)
    add_compile_definitions(__F16C__)
    
    # Linker optimizations
    add_link_options(/LTCG:INCREMENTAL) # Incremental LTCG
    add_link_options(/OPT:REF)          # Remove unused functions
    add_link_options(/OPT:ICF)          # Fold identical functions
else()
    # GCC/Clang SIMD flags - enable all available
    add_compile_options(-msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2)
    add_compile_options($<$<CONFIG:Release>:-ffast-math>)
endif()

# Set optimization levels per configuration
if(PYNOVAGE_COMPILER_MSVC)
    # Debug configuration
    add_compile_options($<$<CONFIG:Debug>:/Od>)   # Disable optimization
    add_compile_options($<$<CONFIG:Debug>:/Ob0>)  # Disable inlining
    
    # Enable Multi-processor compilation
    add_compile_options(/MP)
    
    # Code generation optimizations
    add_compile_options(/Gy)     # Function-level linking
    add_compile_options(/GA)     # Optimize for Windows Application
    add_compile_options(/GF)     # String pooling
    add_compile_options(/Gw)     # Global data optimization
    add_compile_options(/Zc:inline) # Remove unreferenced functions
else()
    # GCC/Clang optimizations
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_options($<$<CONFIG:Debug>:-O0 -g>)
endif()

# Check for LTO support
check_ipo_supported(RESULT LTO_SUPPORTED)
if(LTO_SUPPORTED AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "IPO/LTO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Platform-specific options
if(ANDROID)
    # Android-specific compiler flags
    add_compile_options(-fPIC)
elseif(WIN32)
    # Windows-specific compiler flags
    if(PYNOVAGE_COMPILER_MSVC)
        add_compile_options(/EHsc)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    endif()
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Check for sanitizer support
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(PYNOVAGE_COMPILER_GCC OR PYNOVAGE_COMPILER_CLANG)
        option(PYNOVAGE_ENABLE_ASAN "Enable Address Sanitizer" OFF)
        option(PYNOVAGE_ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" OFF)
        
        if(PYNOVAGE_ENABLE_ASAN)
            add_compile_options(-fsanitize=address)
            add_link_options(-fsanitize=address)
        endif()
        
        if(PYNOVAGE_ENABLE_UBSAN)
            add_compile_options(-fsanitize=undefined)
            add_link_options(-fsanitize=undefined)
        endif()
    endif()
endif()