#================= Setup ==========================
# Utility functions
include(utility)

# Determine component name via folder name
get_filename_component(COMPONENT_NAME_LC "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(TOUPPER ${COMPONENT_NAME_LC} COMPONENT_NAME_UC)
string_proper_case(${COMPONENT_NAME_LC} COMPONENT_NAME)

set(COMPONENT_TARGET_NAME ${COMPONENT_NAME})

# Make lib target
qt_add_library(${COMPONENT_TARGET_NAME} ${COMPONENT_LIB_TYPE})

# Make alias target so target can be referred to with its export namespace
add_library(${CMAKE_PROJECT_NAME}::${COMPONENT_NAME} ALIAS ${COMPONENT_TARGET_NAME})

#================= Build ==========================

# Get timestamp
string(TIMESTAMP BUILD_DATE_TIME "%Y-%m-%d @ %H:%M:%S ")

# Source Files
if(COMPONENT_PRIVATE_HEADERS)
    foreach(p_header ${COMPONENT_PRIVATE_HEADERS})
        target_sources(${COMPONENT_TARGET_NAME} PRIVATE "src/${p_header}")
    endforeach()
endif()

if(COMPONENT_IMPL)
    foreach(impl ${COMPONENT_IMPL})
        target_sources(${COMPONENT_TARGET_NAME} PRIVATE "src/${impl}")
    endforeach()
endif()

# Includes
if(${COMPONENT_LIB_TYPE} STREQUAL "INTERFACE")
    target_include_directories(${COMPONENT_TARGET_NAME} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include/${COMPONENT_NAME_LC}>
    )
else()
    target_include_directories(${COMPONENT_TARGET_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include/${COMPONENT_NAME_LC}>
    )
endif()

if(COMPONENT_INCLUDE_HEADERS)
    # Build pathed include file list
    foreach(api_header ${COMPONENT_INCLUDE_HEADERS})
        set(pathed_api_headers ${pathed_api_headers} "include/${PROJ_NAME_LC}/${COMPONENT_NAME_LC}/${api_header}")
    endforeach()

    # Group include files with their parent directories stripped
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJ_NAME_LC}/${COMPONENT_NAME_LC}"
        PREFIX "Include Files"
        FILES ${pathed_api_headers}
    )

    # Add include files as private target source so that they aren't built nor marked as a dependency,
    # but are shown with the target in the IDE
    target_sources(${COMPONENT_TARGET_NAME} PRIVATE ${pathed_api_headers})
endif()

# Links
if(COMPONENT_PUBLIC_LINKS)
        target_link_libraries(${COMPONENT_TARGET_NAME} PUBLIC ${COMPONENT_PUBLIC_LINKS})
endif()

if(COMPONENT_PRIVATE_LINKS)
        target_link_libraries(${COMPONENT_TARGET_NAME} PRIVATE ${COMPONENT_PRIVATE_LINKS})
endif()

#-----------Generate Primary Component Header------------
set(PRIM_COMP_HEADER_DEF "${PROJ_NAME_UC}_${COMPONENT_NAME_UC}_H")

# Generate include statements
foreach(api_header ${COMPONENT_INCLUDE_HEADERS})
    set(PRIM_COMP_HEADER_INCLUDES "${PRIM_COMP_HEADER_INCLUDES}#include \"${COMPONENT_NAME_LC}/${api_header}\"\n")
endforeach()

# Copy template with modifications
configure_file(
    "${FILE_TEMPLATES_DIR}/primary_component_header.h.in"
    "${CMAKE_BINARY_DIR}/include/${COMPONENT_NAME_LC}.h"
    @ONLY
    NEWLINE_STYLE UNIX
)

#================= Install ==========================

# Configure properties
set_target_properties(${COMPONENT_TARGET_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${COMPONENT_NAME}"
    DEBUG_POSTFIX "d"
)

# Install lib
install(TARGETS ${COMPONENT_TARGET_NAME}
    EXPORT ${COMPONENT_NAME}Targets
    COMPONENT ${COMPONENT_NAME}
    LIBRARY DESTINATION ${STATIC_LIB_INSTALL_DIR_NAME}
    ARCHIVE DESTINATION ${STATIC_LIB_INSTALL_DIR_NAME}
    RUNTIME DESTINATION ${SHARED_LIB_INSTALL_DIR_NAME} # For potential future shared version
)

# Install public headers
install(DIRECTORY ${HEADER_INSTALL_DIR_NAME}/${PROJ_NAME_LC}
    DESTINATION "${HEADER_INSTALL_DIR_NAME}/${COMPONENT_NAME_LC}"
)
install(FILES "${CMAKE_BINARY_DIR}/include/${COMPONENT_NAME_LC}.h"
    DESTINATION "${HEADER_INSTALL_PREFIX}/${COMPONENT_NAME_LC}/${PROJ_NAME_LC}"
)

# Configure package target export
install(EXPORT ${COMPONENT_NAME}Targets
    FILE "${CMAKE_PROJECT_NAME}${COMPONENT_NAME}Targets.cmake"
    NAMESPACE ${CMAKE_PROJECT_NAME}::
    DESTINATION lib/cmake/${CMAKE_PROJECT_NAME}
    COMPONENT ${COMPONENT_NAME}
)
