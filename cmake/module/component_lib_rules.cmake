#================= Setup ==========================
# Utility functions
include(utility)

# Determine component name via folder name
get_filename_component(COMPONENT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(TOUPPER ${COMPONENT_NAME} COMPONENT_NAME_UC)
string_proper_case(${COMPONENT_NAME} COMPONENT_NAME_PROPER)

set(COMPONENT_TARGET_NAME ${COMPONENT_NAME_PROPER})

# Make lib target
qt_add_library(${COMPONENT_TARGET_NAME} ${COMPONENT_LIB_TYPE})

#================= Build ==========================

# Source Files
if(COMPONENT_PUBLIC_API_HEADERS)
        target_sources(${COMPONENT_TARGET_NAME} PRIVATE ${COMPONENT_PUBLIC_API_HEADERS})
endif()

if(COMPONENT_PRIVATE_HEADERS)
        target_sources(${COMPONENT_TARGET_NAME} PRIVATE ${COMPONENT_PRIVATE_HEADERS})
endif()

if(COMPONENT_IMPL)
        target_sources(${COMPONENT_TARGET_NAME} PRIVATE ${COMPONENT_IMPL})
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
    PUBLIC_HEADER "${COMPONENT_PUBLIC_API_HEADERS}"
    VERSION ${CMAKE_PROJECT_VERSION}
    OUTPUT_NAME "${CMAKE_PROJECT_NAME}${EDITION_LETTER}-${COMPONENT_NAME_PROPER}"
    DEBUG_POSTFIX "d"
)

# Determine component sub-path
file(RELATIVE_PATH COMPONENT_SUB_DIR ${SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# Install lib/public headers
install(TARGETS ${COMPONENT_TARGET_NAME}
    LIBRARY DESTINATION ${STATIC_LIB_INSTALL_DIR_NAME}
    ARCHIVE DESTINATION ${STATIC_LIB_INSTALL_DIR_NAME}
    RUNTIME DESTINATION ${SHARED_LIB_INSTALL_DIR_NAME} # For potential future shared version
    PUBLIC_HEADER DESTINATION "${HEADER_INSTALL_DIR_NAME}/${COMPONENT_SUB_DIR}"
)

#-----------Install include group headers------------

# Set variables in install context
install(CODE "set(CONFIG_FILE_DIR \"${CONFIG_FILE_DIR}\")")
install(CODE "set(COMPONENT_NAME \"${COMPONENT_NAME}\")")
install(CODE "set(INCLUDE_GROUP_NAME \"${PROJ_SHORT_NAME_UC}_${COMPONENT_NAME_UC}_H\")")
install(CODE "set(INCLUDE_GROUP_PATH \"${HEADER_INSTALL_PREFIX}/${PROJ_SHORT_NAME}/${COMPONENT_NAME}.h\")")
install(CODE "set(COMPONENT_PUBLIC_API_HEADERS \"${COMPONENT_PUBLIC_API_HEADERS}\")")

# Generate install group header content and apply to template
install(CODE [[
    # Generate include statements
    foreach(apih ${COMPONENT_PUBLIC_API_HEADERS})
        set(INCLUDE_GROUP_FILES "${INCLUDE_GROUP_FILES}#include \"${COMPONENT_NAME}/${apih}\"\n")
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
    unset(COMPONENT_NAME)
    unset(INCLUDE_GROUP_NAME)
    unset(INCLUDE_GROUP_PATH)
    unset(COMPONENT_PUBLIC_API_HEADERS)
    ]]
)
