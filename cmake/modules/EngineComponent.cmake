# Engine component configuration module

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

# Function to add a new engine component library
function(add_engine_component name)
    set(options SHARED STATIC INTERFACE INTERNAL)
    set(oneValueArgs TYPE FOLDER)
    set(multiValueArgs 
        SOURCES 
        PUBLIC_HEADERS 
        PRIVATE_HEADERS 
        PUBLIC_DEPS 
        PRIVATE_DEPS
        FEATURES
    )
    cmake_parse_arguments(COMPONENT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Determine library type
    if(COMPONENT_SHARED)
        set(lib_type SHARED)
    elseif(COMPONENT_STATIC)
        set(lib_type STATIC)
    elseif(COMPONENT_INTERFACE)
        set(lib_type INTERFACE)
    else()
        set(lib_type STATIC)
    endif()

    # Create the library
    if(COMPONENT_INTERFACE)
        add_library(${name} INTERFACE)
    else()
        add_library(${name} ${lib_type}
            ${COMPONENT_SOURCES}
            ${COMPONENT_PUBLIC_HEADERS}
            ${COMPONENT_PRIVATE_HEADERS}
        )
    endif()

    # Add alias target
    add_library(PyNovaGE::${name} ALIAS ${name})

    # Set target properties
    if(NOT COMPONENT_INTERFACE)
        set_target_properties(${name} PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
            POSITION_INDEPENDENT_CODE ON
            DEBUG_POSTFIX "d"
            FOLDER "${COMPONENT_FOLDER}"
        )
    endif()

    # Generate export header if not interface
    if(NOT COMPONENT_INTERFACE)
        generate_export_header(${name}
            EXPORT_FILE_NAME include/${name}/export.hpp
            EXPORT_MACRO_NAME ${name}_API
            NO_EXPORT_MACRO_NAME ${name}_INTERNAL
            DEPRECATED_MACRO_NAME ${name}_DEPRECATED
        )
    endif()

    # Set include directories
    if(COMPONENT_INTERFACE)
        target_include_directories(${name} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
    else()
        target_include_directories(${name}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
                $<INSTALL_INTERFACE:include>
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )
    endif()

    # Link dependencies
    if(COMPONENT_PUBLIC_DEPS)
        target_link_libraries(${name}
            PUBLIC ${COMPONENT_PUBLIC_DEPS}
        )
    endif()

    if(COMPONENT_PRIVATE_DEPS)
        target_link_libraries(${name}
            PRIVATE ${COMPONENT_PRIVATE_DEPS}
        )
    endif()

    # Add platform-specific settings
    if(NOT COMPONENT_INTERFACE)
        set_platform_definitions(${name})
        target_link_platform_libraries(${name})
        target_include_platform_directories(${name})
        add_platform_sources(${name})
    endif()

    # Setup installation
    if(NOT COMPONENT_INTERNAL)
        install(TARGETS ${name}
            EXPORT PyNovaGETargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

        # Install headers
        if(COMPONENT_PUBLIC_HEADERS)
            foreach(header ${COMPONENT_PUBLIC_HEADERS})
                get_filename_component(header_path ${header} PATH)
                install(FILES ${header}
                    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${name}/${header_path}
                )
            endforeach()
        endif()

        # Install export header
        if(NOT COMPONENT_INTERFACE)
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/${name}/export.hpp
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${name}
            )
        endif()
    endif()

    # Add tests if enabled
    if(PYNOVAGE_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
        add_subdirectory(tests)
    endif()

    # Add benchmarks if enabled
    if(PYNOVAGE_BUILD_BENCHMARKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks")
        add_subdirectory(benchmarks)
    endif()
endfunction()

# Function to create an engine module (collection of components)
function(add_engine_module name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs COMPONENTS)
    cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Create interface library for the module
    add_library(${name} INTERFACE)
    add_library(PyNovaGE::${name} ALIAS ${name})

    # Link all components
    if(MODULE_COMPONENTS)
        target_link_libraries(${name} INTERFACE ${MODULE_COMPONENTS})
    endif()

    # Install module interface
    install(TARGETS ${name}
        EXPORT PyNovaGETargets
    )
endfunction()

# Function to setup component documentation
function(add_component_documentation name)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs/${name})
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_XML YES)

        doxygen_add_docs(${name}_docs
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            COMMENT "Generating documentation for ${name}"
        )
    endif()
endfunction()