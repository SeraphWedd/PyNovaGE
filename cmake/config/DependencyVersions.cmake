# Dependency versions configuration

# Testing and benchmarking
set(GTEST_VERSION "v1.14.0")
set(GBENCHMARK_VERSION "v1.8.3")

# Graphics APIs minimum versions
set(VULKAN_MIN_VERSION "1.2")
set(OPENGL_MIN_VERSION "4.5")
set(GLES_MIN_VERSION "3.2")
set(DX12_MIN_VERSION "12.0")

# Platform-specific dependencies
if(ANDROID)
    set(ANDROID_MIN_SDK 29)      # Android 10.0
    set(ANDROID_TARGET_SDK 33)   # Android 13.0
    set(NDK_MIN_VERSION "25.0.0")
endif()

# Version check functions
function(check_vulkan_version)
    if(Vulkan_FOUND)
        if(Vulkan_VERSION VERSION_LESS VULKAN_MIN_VERSION)
            message(WARNING "Found Vulkan version ${Vulkan_VERSION} is less than required ${VULKAN_MIN_VERSION}")
        endif()
    endif()
endfunction()

function(check_opengl_version)
    if(OPENGL_FOUND)
        if(OPENGL_VERSION VERSION_LESS OPENGL_MIN_VERSION)
            message(WARNING "Found OpenGL version ${OPENGL_VERSION} is less than required ${OPENGL_MIN_VERSION}")
        endif()
    endif()
endfunction()

function(check_gles_version)
    if(GLES3_FOUND)
        if(GLES3_VERSION VERSION_LESS GLES_MIN_VERSION)
            message(WARNING "Found OpenGL ES version ${GLES3_VERSION} is less than required ${GLES_MIN_VERSION}")
        endif()
    endif()
endfunction()

function(check_dx12_version)
    if(D3D12_FOUND)
        if(D3D12_VERSION VERSION_LESS DX12_MIN_VERSION)
            message(WARNING "Found DirectX 12 version ${D3D12_VERSION} is less than required ${DX12_MIN_VERSION}")
        endif()
    endif()
endfunction()