function(__qx_get_component_path component return)
    string(TOLOWER "${component}" COMPONENT_LC)
    set(COMPONENT_PATH "${PROJECT_SOURCE_DIR}/comp/${COMPONENT_LC}")

    if(NOT EXISTS "${COMPONENT_PATH}")
        message(FATAL_ERROR "${component} is not the name of a Qx component!")
    endif()

    set(${return} "${COMPONENT_PATH}" PARENT_SCOPE)
endfunction()

function(__qx_get_depends_from_file file_path return)
    if(EXISTS "${file_path}")
        file(STRINGS "${file_path}" DEPENDS REGEX [[[^ \t\v\r\n]+]])
    endif()

    set(${return} "${DEPENDS}" PARENT_SCOPE)
endfunction()

function(__qx_split_header_types headers r_private r_public)
    # Default to public
    set(CURRENT_TYPE "PUBLIC")

    # Split
    foreach(header ${headers})
        if(("${header}" STREQUAL "PUBLIC") OR ("${header}" STREQUAL "PRIVATE"))
            set(CURRENT_TYPE ${header})
        elseif(${CURRENT_TYPE} STREQUAL "PUBLIC")
            list(APPEND PUBLIC_HEADERS ${header})
        elseif(${CURRENT_TYPE} STREQUAL "PRIVATE")
            list(APPEND PRIVATE_HEADERS ${header})
        endif()
    endforeach()

    set(${r_private} "${PRIVATE_HEADERS}" PARENT_SCOPE)
    set(${r_public} "${PUBLIC_HEADERS}" PARENT_SCOPE)
endfunction()

function(qx_get_component_qt_depends component return)
    __qx_get_component_path("${component}" COMPONENT_PATH)
    __qx_get_depends_from_file("${COMPONENT_PATH}/cmake/depends_qt" DEPENDS_QT)
    set(${return} "${DEPENDS_QT}" PARENT_SCOPE)
endfunction()

function(qx_get_component_sibling_depends component return)
    __qx_get_component_path("${component}" COMPONENT_PATH)
    __qx_get_depends_from_file("${COMPONENT_PATH}/cmake/depends_siblings" DEPENDS_SIBLINGS)
    set(${return} "${DEPENDS_SIBLINGS}" PARENT_SCOPE)
endfunction()

function(qx_get_all_qt_depends components return)
    foreach(component ${components})
        qx_get_component_qt_depends("${component}" QT_DEPENDS)
        list(APPEND ALL_QT_DEPENDS ${QT_DEPENDS})
    endforeach()

    list(REMOVE_DUPLICATES ALL_QT_DEPENDS)
    set(${return} "${ALL_QT_DEPENDS}" PARENT_SCOPE)
endfunction()

function(qx_enumerate_sibling_tree components return)
    foreach(component ${components})
        # Ensure consistant case to avoid duplicates in list
        string_to_proper_case("${component}" COMPONENT_PC)

        # Check if component has already been processed
        if(COMPONENT_PC IN_LIST FULL_COMPONENTS)
            continue()
        endif()

        # Add component to final list
        list(APPEND FULL_COMPONENTS "${COMPONENT_PC}")

        # Get siblings
        qx_get_component_sibling_depends("${COMPONENT_PC}" SIBLING_COMPONENTS)

        # Recurse
        qx_enumerate_sibling_tree("${SIBLING_COMPONENTS}" FULL_COMPONENTS)
    endforeach()

    # Return list at depth
    set(${return} "${FULL_COMPONENTS}" PARENT_SCOPE)
endfunction()

function(qx_register_component)
    #=============== Parse Arguments ==================

    # Function inputs
    set(oneValueArgs
        NAME
        LIB_TYPE
    )
    set(multiValueArgs
        DEPENDS_SIBLINGS
        DEPENDS_QT
        HEADERS
        IMPLEMENTATION
        DOC_ONLY
        LINKS
    )

    # Parse arguments
    cmake_parse_arguments(COMPONENT "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate input
    foreach(unk_val ${COMPONENT_UNPARSED_ARGUMENTS})
        message(WARNING "Ignoring unrecognized parameter: ${unk_val}")
    endforeach()

    if(COMPONENT_KEYWORDS_MISSING_VALUES)
        foreach(missing_val ${COMPONENT_KEYWORDS_MISSING_VALUES})
            # Qt and sibling depend vars can be missing values
            if((NOT "${missing_val}" STREQUAL "DEPENDS_SIBLINGS") AND (NOT "${missing_val}" STREQUAL "DEPENDS_QT"))
                message(WARNING "A value for '${missing_val}' must be provided")
                set(REQUIRED_VALUE_MISSING TRUE)
            endif()
        endforeach()
        if(REQUIRED_VALUE_MISSING)
            message(FATAL_ERROR "Not all required values were present!")
        endif()
    endif()

    # Handle defaults/undefineds
    if(NOT DEFINED COMPONENT_NAME)
        message(FATAL_ERROR "A name for the component must be provided!")
    endif()
    if(NOT DEFINED COMPONENT_LIB_TYPE)
        message(FATAL_ERROR "A lib type for the component must be provided!")
    endif()

    #============ Additional Variable Prep ============

    string(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME_LC)
    string(TOUPPER ${COMPONENT_NAME} COMPONENT_NAME_UC)
    __qx_split_header_types("${COMPONENT_HEADERS}" COMPONENT_HEADERS_PRIVATE COMPONENT_HEADERS_PUBLIC)

    #================= Setup ==========================

    create_header_guard(${PROJECT_NAME} ${COMPONENT_NAME} COMPONENT_HEADER_GUARD)

    # Name here needs to be as unique as possible for when this project is inlcuded
    # in another via FetchContent or add_subdirectory (prevent target clashes)
    set(COMPONENT_TARGET_NAME ${PROJECT_NAME_LC}_${COMPONENT_NAME_LC})

    # Make lib target
    qt_add_library(${COMPONENT_TARGET_NAME} ${COMPONENT_LIB_TYPE})

    # Make alias target so target can be referred to with its friendly
    # export name both internally and when part of another build tree
    add_library(${PROJECT_NAME}::${COMPONENT_NAME} ALIAS ${COMPONENT_TARGET_NAME})

    #================= Build ==========================

    # Get timestamp
    string(TIMESTAMP BUILD_DATE_TIME "%Y-%m-%d @ %H:%M:%S ")

    # Source Files
    if(COMPONENT_HEADERS_PRIVATE)
        foreach(p_header ${COMPONENT_HEADERS_PRIVATE})
            target_sources(${COMPONENT_TARGET_NAME} PRIVATE "src/${p_header}")
        endforeach()
    endif()

    if(COMPONENT_IMPLEMENTATION)
        foreach(impl ${COMPONENT_IMPL})
            # Ignore not relavent system specific implementation
            string(REGEX MATCH [[_win\.cpp$]] IS_WIN_IMPL "${impl}")
            string(REGEX MATCH [[_linux\.cpp$]] IS_LINUX_IMPL "${impl}")
            if((IS_WIN_IMPL AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows") OR
               (IS_LINUX_IMPL AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux"))
                continue()
            endif()

            target_sources(${COMPONENT_TARGET_NAME} PRIVATE "src/${impl}")
        endforeach()
    endif()

    # Includes
    if(${COMPONENT_LIB_TYPE} STREQUAL "INTERFACE")
        target_include_directories(${COMPONENT_TARGET_NAME} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include/${COMPONENT_NAME_LC}>
        )
    else()
        target_include_directories(${COMPONENT_TARGET_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include/${COMPONENT_NAME_LC}>
        )
    endif()

    if(COMPONENT_HEADERS_PUBLIC)
        # Build pathed include file list
        foreach(api_header ${COMPONENT_HEADERS_PUBLIC})
            set(pathed_api_headers ${pathed_api_headers} "include/${PROJECT_NAME_LC}/${COMPONENT_NAME_LC}/${api_header}")
        endforeach()

        # Group include files with their parent directories stripped
        source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME_LC}/${COMPONENT_NAME_LC}"
            PREFIX "Include Files"
            FILES ${pathed_api_headers}
        )

        # Add include files as private target source so that they aren't built nor marked as a dependency,
        # but are shown with the target in the IDE
        target_sources(${COMPONENT_TARGET_NAME} PRIVATE ${pathed_api_headers})
    endif()

    # Doc
    if(COMPONENT_DOC_ONLY)
        # Build pathed include file list
        foreach(doc ${COMPONENT_DOC_ONLY})
            set(pathed_docs ${pathed_docs} "src/${doc}")
        endforeach()

        # Group include files with their parent directories stripped
        source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
            PREFIX "Doc Files"
            FILES ${pathed_docs}
        )

        # Add include files as private target source so that they aren't built nor marked as a dependency,
        # but are shown with the target in the IDE
        target_sources(${COMPONENT_TARGET_NAME} PRIVATE ${pathed_docs})
    endif()

    # Links
    if(COMPONENT_LINKS)
        target_link_libraries(${COMPONENT_TARGET_NAME} ${COMPONENT_LINKS})
    endif()

    #-----------Generate Primary Component Header------------

    # Generate include statements
    foreach(api_header ${COMPONENT_HEADERS_PUBLIC})
        set(PRIM_COMP_HEADER_INCLUDES "${PRIM_COMP_HEADER_INCLUDES}#include <${PROJECT_NAME_LC}/${COMPONENT_NAME_LC}/${api_header}>\n")
    endforeach()

    # Copy template with modifications
    configure_file(
        "${FILE_TEMPLATES_PATH}/primary_component_header.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME_LC}/${COMPONENT_NAME_LC}.h"
        @ONLY
        NEWLINE_STYLE UNIX
    )

    #--------------------Package Config-----------------------

    # Create config file
    configure_file("${FILE_TEMPLATES_PATH}/${PROJECT_NAME}ComponentConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/cmake/${COMPONENT_NAME}/${PROJECT_NAME}${COMPONENT_NAME}Config.cmake"
        @ONLY
    )

    #---------- Configure Target Properties------------------
    set_target_properties(${COMPONENT_TARGET_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        OUTPUT_NAME "${PROJECT_NAME}-${COMPONENT_NAME}"
        DEBUG_POSTFIX "d"
        EXPORT_NAME "${COMPONENT_NAME}"
    )

    #================= Install ==========================

    # Install component lib
    install(TARGETS ${COMPONENT_TARGET_NAME}
        COMPONENT ${COMPONENT_TARGET_NAME}
        EXPORT ${COMPONENT_NAME}Targets
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin # For potential future shared version
    )

    # Install public headers
    install(DIRECTORY include/
        COMPONENT ${COMPONENT_TARGET_NAME}
        DESTINATION "include/${COMPONENT_NAME_LC}/"
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME_LC}/${COMPONENT_NAME_LC}.h"
        COMPONENT ${COMPONENT_TARGET_NAME}
        DESTINATION "include/${COMPONENT_NAME_LC}/${PROJECT_NAME_LC}"
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
    )

    # Install package target export
    install(EXPORT ${COMPONENT_NAME}Targets
        COMPONENT ${COMPONENT_TARGET_NAME}
        FILE "${PROJECT_NAME}${COMPONENT_NAME}Targets.cmake"
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION cmake/${COMPONENT_NAME}
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
    )

    # Install package config
    install(FILES
        "${PROJECT_BINARY_DIR}/cmake/${COMPONENT_NAME}/${PROJECT_NAME}${COMPONENT_NAME}Config.cmake"
        COMPONENT ${COMPONENT_TARGET_NAME}
        DESTINATION cmake/${COMPONENT_NAME}
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
    )

    #========Export For In-tree Builds =================
    # For in source builds
    export(EXPORT ${COMPONENT_NAME}Targets
        FILE "${PROJECT_BINARY_DIR}/cmake/${COMPONENT_NAME}/${PROJECT_NAME}${COMPONENT_NAME}Targets.cmake"
        NAMESPACE ${PROJECT_NAME}::
    )
endfunction()
