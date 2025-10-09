# Packaging configuration

include(CPack)

# Basic package information
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Your Organization")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Package components
set(CPACK_COMPONENTS_ALL
    runtime     # Runtime libraries
    devel       # Development files
    python      # Python bindings
    tools       # Development tools
    examples    # Example applications
    docs        # Documentation
)

set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime Libraries")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime libraries required to run applications using ${PROJECT_NAME}")
set(CPACK_COMPONENT_RUNTIME_REQUIRED TRUE)

set(CPACK_COMPONENT_DEVEL_DISPLAY_NAME "Development Files")
set(CPACK_COMPONENT_DEVEL_DESCRIPTION "Files required to build applications using ${PROJECT_NAME}")
set(CPACK_COMPONENT_DEVEL_DEPENDS runtime)

set(CPACK_COMPONENT_PYTHON_DISPLAY_NAME "Python Bindings")
set(CPACK_COMPONENT_PYTHON_DESCRIPTION "Python bindings for ${PROJECT_NAME}")
set(CPACK_COMPONENT_PYTHON_DEPENDS runtime)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Development Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "Tools for ${PROJECT_NAME} development")
set(CPACK_COMPONENT_TOOLS_DEPENDS devel)

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Example applications using ${PROJECT_NAME}")
set(CPACK_COMPONENT_EXAMPLES_DEPENDS runtime)

set(CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOCS_DESCRIPTION "Documentation for ${PROJECT_NAME}")

# Platform-specific packaging
if(WIN32)
    # Windows installer using WiX
    set(CPACK_GENERATOR "WIX")
    set(CPACK_WIX_UPGRADE_GUID "12345678-1234-1234-1234-123456789012")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico")
    set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/resources/banner.png")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/resources/dialog.png")
    
    # Add to PATH
    set(CPACK_NSIS_MODIFY_PATH ON)
    
    # Registry keys
    set(CPACK_WIX_PROGRAM_MENU_FOLDER ${PROJECT_NAME})
    
elseif(APPLE)
    # macOS bundle
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
    set(CPACK_DMG_FORMAT "UDZO")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/resources/background.png")
    
elseif(UNIX)
    # Linux packages
    set(CPACK_GENERATOR "DEB;RPM")
    
    # DEB-specific
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.17), libstdc++6 (>= 4.8)")
    
    # RPM-specific
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc >= 2.17, libstdc++ >= 4.8")
    
    # Set package architecture
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
    set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
endif()

# Source package
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git/"
    "/\\\\.github/"
    "/build/"
    "/install/"
    "\\\\.DS_Store"
    "\\\\.gitignore"
    "\\\\.gitmodules"
    "/CMakeFiles/"
    "CMakeCache\\\\.txt"
    "\\\\.cmake$"
    "\\\\.user$"
    "\\\\.vscode/"
    "\\\\.vs/"
)