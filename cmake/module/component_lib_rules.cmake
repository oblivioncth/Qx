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
            $<INSTALL_INTERFACE:include>
    )
else()
    target_include_directories(${COMPONENT_TARGET_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
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

#================= Install ==========================

# Configure properties
set_target_properties(${COMPONENT_TARGET_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    OUTPUT_NAME "${CMAKE_PROJECT_NAME}${EDITION_LETTER}-${COMPONENT_NAME}"
    DEBUG_POSTFIX "d"
)

# Install lib/public headers
install(TARGETS ${COMPONENT_TARGET_NAME}
    EXPORT ${COMPONENT_NAME}-targets
    COMPONENT ${COMPONENT_NAME}
    LIBRARY DESTINATION ${STATIC_LIB_INSTALL_DIR_NAME}
    ARCHIVE DESTINATION ${STATIC_LIB_INSTALL_DIR_NAME}
    RUNTIME DESTINATION ${SHARED_LIB_INSTALL_DIR_NAME} # For potential future shared version
)

install(EXPORT ${COMPONENT_NAME}-targets
    FILE "${CMAKE_PROJECT_NAME}-${COMPONENT_NAME}-targets.cmake"
    NAMESPACE ${CMAKE_PROJECT_NAME}::
    DESTINATION lib/cmake/${CMAKE_PROJECT_NAME}
    COMPONENT ${COMPONENT_NAME}
)

#-----------Install include group headers------------

# Set variables in install context
install(CODE "set(CONFIG_FILE_DIR \"${CONFIG_FILE_DIR}\")")
install(CODE "set(COMPONENT_NAME_LC \"${COMPONENT_NAME_LC}\")")
install(CODE "set(INCLUDE_GROUP_NAME \"${PROJ_NAME_UC}_${COMPONENT_NAME_UC}_H\")")
install(CODE "set(INCLUDE_GROUP_PATH \"${HEADER_INSTALL_PREFIX}/${PROJ_NAME_LC}/${COMPONENT_NAME_LC}.h\")")
install(CODE "set(COMPONENT_INCLUDE_HEADERS \"${COMPONENT_INCLUDE_HEADERS}\")")

# Generate install group header content and apply to template
install(CODE [[
    # Generate include statements
    foreach(apih ${COMPONENT_INCLUDE_HEADERS})
        set(INCLUDE_GROUP_FILES "${INCLUDE_GROUP_FILES}#include \"${COMPONENT_NAME_LC}/${apih}\"\n")
    endforeach()

    # Copy template with modifications
    configure_file(
        "${CONFIG_FILE_DIR}/include_group.h"
        "${INCLUDE_GROUP_PATH}"
        NEWLINE_STYLE UNIX
    )

    # Unset variables in install context since they are global there and may reused
    unset(INCLUDE_GROUP_FILES)
    unset(CONFIG_FILE_DIR)
    unset(COMPONENT_NAME_LC)
    unset(INCLUDE_GROUP_NAME)
    unset(INCLUDE_GROUP_PATH)
    unset(COMPONENT_INCLUDE_HEADERS)
    ]]
)

#--------------------Package Config----------------
configure_file("${CONFIG_FILE_DIR}/component-config.cmake.in"
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${COMPONENT_NAME}-config.cmake"
    @ONLY
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${COMPONENT_NAME}-config-version.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY ExactVersion
)

install(FILES
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${COMPONENT_NAME}-config.cmake"
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${COMPONENT_NAME}-config-version.cmake"
    DESTINATION cmake/${CMAKE_PROJECT_NAME}
    COMPONENT ${component}
)
