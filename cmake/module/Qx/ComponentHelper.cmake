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

function(__qx_get_component_qt_depends component return)
    __qx_get_component_path("${component}" COMPONENT_PATH)
    __qx_get_depends_from_file("${COMPONENT_PATH}/cmake/depends_qt" DEPENDS_QT)
    set(${return} "${DEPENDS_QT}" PARENT_SCOPE)
endfunction()

function(__qx_get_component_sibling_depends component return)
    __qx_get_component_path("${component}" COMPONENT_PATH)
    __qx_get_depends_from_file("${COMPONENT_PATH}/cmake/depends_siblings" DEPENDS_SIBLINGS)
    set(${return} "${DEPENDS_SIBLINGS}" PARENT_SCOPE)
endfunction()

function(qx_get_all_qt_depends components return)
    foreach(component ${components})
        __qx_get_component_qt_depends("${component}" QT_DEPENDS)
        list(APPEND ALL_QT_DEPENDS ${QT_DEPENDS})
    endforeach()

    list(REMOVE_DUPLICATES ALL_QT_DEPENDS)
    set(${return} "${ALL_QT_DEPENDS}" PARENT_SCOPE)
endfunction()

function(qx_enumerate_sibling_tree components return)
    foreach(component ${components})
        # Ensure consistant case to avoid duplicates in list
        ob_string_to_proper_case("${component}" COMPONENT_PC)

        # Check if component has already been processed
        if(COMPONENT_PC IN_LIST FULL_COMPONENTS)
            continue()
        endif()

        # Add component to final list
        list(APPEND FULL_COMPONENTS "${COMPONENT_PC}")

        # Get siblings
        __qx_get_component_sibling_depends("${COMPONENT_PC}" SIBLING_COMPONENTS)

        # Recurse
        qx_enumerate_sibling_tree("${SIBLING_COMPONENTS}" FULL_COMPONENTS)
    endforeach()

    # Return list at depth
    set(${return} "${FULL_COMPONENTS}" PARENT_SCOPE)
endfunction()

function(qx_add_component COMPONENT_NAME)
    #=============== Parse Arguments ==================

    # Function inputs
    set(multiValueArgs
        HEADERS_PRIVATE
        HEADERS_API
        IMPLEMENTATION
        DOC_ONLY
        LINKS
    )
    set(requiredArgs
        NAME
        HEADERS_API
    )

    # Parse arguments
    ob_parse_arguments(COMPONENT "" "${oneValueArgs}" "${multiValueArgs}" "${requiredArgs}" ${ARGN})

    # -------------------- Basic Setup ---------------------
    # Create derivative values
    string(TOLOWER "${COMPONENT_NAME}" COMPONENT_NAME_LC)
    string(TOUPPER "${COMPONENT_NAME}" COMPONENT_NAME_UC)

    # Automatically detect interface library
    if(COMPONENT_IMPLEMENTATION)
        set(COMPONENT_TYPE "")
    else()
        set(COMPONENT_TYPE "INTERFACE")
    endif()

    # ---------- Generate Primary Include Header ------------
    # Get Date/Time
    string(TIMESTAMP BUILD_DATE_TIME "%Y-%m-%d @ %H:%M:%S ")

    # Create header guard
    ob_create_header_guard(${PROJECT_NAMESPACE} ${COMPONENT_NAME} COMPONENT_HEADER_GUARD)

    # Generate include list
    set(COMPONENT_HEADER_INCLUDES "") # Avoid unused warning
    foreach(api_header ${COMPONENT_HEADERS_API})
        set(COMPONENT_HEADER_INCLUDES "${COMPONENT_HEADER_INCLUDES}#include <${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}/${api_header}>\n")
    endforeach()

    # Copy template with modifications
    set(gen_output_rel_path "${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}.h")
    configure_file(
        "${PROJECT_FILE_TEMPLATES}/primary_component_header.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/include/${gen_output_rel_path}"
        @ONLY
        NEWLINE_STYLE UNIX
    )

    # ---------- Prepare Target Config Generation ------------
    __qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_DEPENDS_QT)
    __qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_DEPENDS_SIBLINGS)

    # --------------------- Add Library -----------------------
    include(OB/Library)
    ob_add_standard_library("${PROJECT_NAMESPACE_LC}_${COMPONENT_NAME_LC}"
        NAMESPACE "${PROJECT_NAMESPACE}"
        ALIAS "${COMPONENT_NAME}"
        TYPE "${COMPONENT_TYPE}"
        EXPORT_HEADER
            PATH "${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}/${PROJECT_NAMESPACE_LC}_${COMPONENT_NAME_LC}_export.h"
        HEADERS_PRIVATE
            ${COMPONENT_HEADERS_PRIVATE}
        HEADERS_API
            COMMON "${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}"
            FILES ${COMPONENT_HEADERS_API}
        HEADERS_API_GEN
            FILES ${gen_output_rel_path}
        IMPLEMENTATION
            ${COMPONENT_IMPLEMENTATION}
        DOC_ONLY
            ${COMPONENT_DOC_ONLY}
        LINKS
            ${COMPONENT_LINKS}
        CONFIG CUSTOM "${PROJECT_FILE_TEMPLATES}/${PROJECT_NAMESPACE}ComponentConfig.cmake.in"
    )
endfunction()
