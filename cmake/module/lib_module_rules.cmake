#================= Setup ==========================
# Determine module name via folder name
get_filename_component(MODULE_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(TOUPPER ${MODULE_NAME} MODULE_NAME_UC)

set(MODULE_TARGET_NAME ${PROJ_SHORT_NAME}-${MODULE_NAME})

# Make lib target
qt_add_library(${MODULE_TARGET_NAME} ${MODULE_TYPE})

#================= Build ==========================

# Source Files
if(MODULE_PUBLIC_HEADERS)
        target_sources(${MODULE_TARGET_NAME} PUBLIC ${MODULE_PUBLIC_HEADERS})
endif()

if(MODULE_PRIVATE_HEADERS)
        target_sources(${MODULE_TARGET_NAME} PRIVATE ${MODULE_PRIVATE_HEADERS})
endif()

if(MODULE_IMPL)
        target_sources(${MODULE_TARGET_NAME} PRIVATE ${MODULE_IMPL})
endif()

# Links
if(MODULE_PUBLIC_LINKS)
        target_link_libraries(${MODULE_TARGET_NAME} PUBLIC ${MODULE_PUBLIC_LINKS})
endif()

if(MODULE_PRIVATE_LINKS)
        target_link_libraries(${MODULE_TARGET_NAME} PRIVATE ${MODULE_PUBLIC_LINKS})
endif()

#================= Install ==========================

# Configure properties
set_target_properties(${MODULE_TARGET_NAME} PROPERTIES
    PUBLIC_HEADER "${MODULE_PUBLIC_HEADERS}"
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    DEBUG_POSTFIX "d"
)

# Determine module sub-path
file(RELATIVE_PATH MODULE_SUB_DIR ${SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# Install lib/public headers
install(TARGETS ${MODULE_TARGET_NAME}
    LIBRARY DESTINATION ${STATIC_LIB_INSTALL_DIR_NAME}
    ARCHIVE DESTINATION ${STATIC_LIB_INSTALL_DIR_NAME}
    RUNTIME DESTINATION ${SHARED_LIB_INSTALL_DIR_NAME}
    PUBLIC_HEADER DESTINATION "${HEADER_INSTALL_DIR_NAME}/${MODULE_SUB_DIR}"
)

#-----------Install include group headers------------

# Set variables in install context
install(CODE "set(CONFIG_FILE_DIR \"${CONFIG_FILE_DIR}\")")
install(CODE "set(MODULE_NAME \"${MODULE_NAME}\")")
install(CODE "set(INCLUDE_GROUP_NAME \"${PROJ_SHORT_NAME_UC}_${MODULE_NAME_UC}_H\")")
install(CODE "set(INCLUDE_GROUP_PATH \"${HEADER_INSTALL_PREFIX}/${PROJ_SHORT_NAME}/${MODULE_NAME}.h\")")
install(CODE "set(MODULE_PUBLIC_HEADERS \"${MODULE_PUBLIC_HEADERS}\")")

# Generate install group header content and apply to template
install(CODE [[
    # Generate include statements
    foreach(apih ${MODULE_PUBLIC_HEADERS})
        set(INCLUDE_GROUP_FILES "${INCLUDE_GROUP_FILES}#include \"${MODULE_NAME}/${apih}\"\n")
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
    unset(MODULE_NAME)
    unset(INCLUDE_GROUP_NAME)
    unset(INCLUDE_GROUP_PATH)
    unset(MODULE_PUBLIC_HEADERS)
    ]]
)
