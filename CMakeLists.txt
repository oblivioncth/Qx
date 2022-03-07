#================= Project Setup ==========================
# CMake
cmake_minimum_required(VERSION 3.21.1)

# Project
project(Qx VERSION 0.1.0.0 LANGUAGES CXX)
string(TOLOWER ${CMAKE_PROJECT_NAME} PROJ_NAME_LC)
string(TOUPPER ${CMAKE_PROJECT_NAME} PROJ_NAME_UC)

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build augmentation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Disable deprecated code
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Directory Names
set(CMAKE_UTIL_DIR_NAME cmake)
set(CMAKE_MODULE_DIR_NAME module)
set(CONFIG_FILE_DIR_NAME config_file)
set(COMPONENT_DIR_NAME comp)
set(INSTALL_DIR_NAME dist)
set(STATIC_LIB_INSTALL_DIR_NAME lib)
set(SHARED_LIB_INSTALL_DIR_NAME bin)
set(HEADER_INSTALL_DIR_NAME include)

# Paths
set(CMAKE_UTIL_DIR "${CMAKE_SOURCE_DIR}/${CMAKE_UTIL_DIR_NAME}")
set(CMAKE_MODULE_PATH "${CMAKE_UTIL_DIR}/${CMAKE_MODULE_DIR_NAME}")
set(CONFIG_FILE_DIR "${CMAKE_UTIL_DIR}/${CONFIG_FILE_DIR_NAME}")
set(COMPONENTS_DIR "${CMAKE_SOURCE_DIR}/${COMPONENT_DIR_NAME}")
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/${INSTALL_DIR_NAME}/${CMAKE_BUILD_TYPE}")
set(HEADER_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${HEADER_INSTALL_DIR_NAME}")

# Clean install when clean target is ran
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_INSTALL_PREFIX}")

################# Common Build #################
set(LIB_COMPONENTS
    core
    gui
    io
    utility
    network
    xml
)

# Qt package components
set(QT_FIND_COMPONENTS
    Core
    Gui
    Network
    Widgets
    Xml
)

################# Windows Build #################
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    list(APPEND LIB_COMPONENTS windows)
endif()

################# Linux Build #################
if(CMAKE_SYSTEM_NAME STREQUAL Linux)

endif()

#================= Component Setup ==========================
# Find Qt package
find_package(Qt6 REQUIRED COMPONENTS ${QT_FIND_COMPONENTS})

# Build components
foreach(component ${LIB_COMPONENTS})
    add_subdirectory(${COMPONENTS_DIR}/${component})
endforeach()

#================= Install ===============================

#-----------Install main include header------------

# Set variables in install/code context
install(CODE "set(CONFIG_FILE_DIR \"${CONFIG_FILE_DIR}\")")
install(CODE "set(LIB_NAME \"${CMAKE_PROJECT_NAME}\")")
install(CODE "set(LIB_NAME_UC \"${PROJ_NAME_UC}\")")
install(CODE "set(LIB_VER_MAJOR \"${CMAKE_PROJECT_VERSION_MAJOR}\")")
install(CODE "set(LIB_VER_MINOR \"${CMAKE_PROJECT_VERSION_MINOR}\")")
install(CODE "set(LIB_VER_REV \"${CMAKE_PROJECT_VERSION_PATCH}\")")
install(CODE "set(LIB_VER_BUILD \"${CMAKE_PROJECT_VERSION_TWEAK}\")")
install(CODE "string(CONCAT LIB_VER ${CMAKE_PROJECT_VERSION_MAJOR}
                                    ${CMAKE_PROJECT_VERSION_MINOR}
                                    ${CMAKE_PROJECT_VERSION_PATCH}
                                    ${CMAKE_PROJECT_VERSION_TWEAK})")
install(CODE "set(QT_VER \"${Qt6_VERSION}\")")
install(CODE "set(PLATFORM \"${CMAKE_SYSTEM_NAME}\")")
install(CODE "set(PROJ_NAME_LC \"${PROJ_NAME_LC}\")")
install(CODE "set(MAIN_INCLUDE_NAME \"${PROJ_NAME_UC}_H\")")
install(CODE "set(MAIN_INCLUDE_PATH \"${HEADER_INSTALL_PREFIX}/${PROJ_NAME_LC}.h\")")
install(CODE "set(COMPONENT_INCLUDE_DIR \"${HEADER_INSTALL_PREFIX}/${PROJ_NAME_LC}\")")

# Generate main public include header
install(CODE [[
    # Glob module include groups
    file(GLOB COMPONENT_INCLUDE_GROUPS
        RELATIVE "${COMPONENT_INCLUDE_DIR}"
        "${COMPONENT_INCLUDE_DIR}/*.h"
    )

    # Generate include statements
    foreach(comp_ig ${COMPONENT_INCLUDE_GROUPS})
        set(COMPONENT_INCLUDE_FILES "${COMPONENT_INCLUDE_FILES}#include \"${PROJ_NAME_LC}/${comp_ig}\"\n")
    endforeach()

    # Get timestamp
    string(TIMESTAMP BUILD_DATE "%Y-%m-%d @ %H:%M:%S ")

    # Copy template with modifications
    configure_file(
        "${CONFIG_FILE_DIR}/main_include.h"
        "${MAIN_INCLUDE_PATH}"
        NEWLINE_STYLE UNIX
    )

    # Unset variables in install context since they are global there and may reused
    unset(COMPONENT_INCLUDE_FILES)
    unset(BUILD_DATE)
    unset(CONFIG_FILE_DIR)
    unset(LIB_NAME)
    unset(LIB_NAME_UC)
    unset(LIB_VER)
    unset(LIB_VER_MAJOR)
    unset(LIB_VER_MINOR)
    unset(LIB_VER_REV)
    unset(LIB_VER_BUILD)
    unset(QT_VER)
    unset(PLATFORM)
    unset(BUILD_DATE)
    unset(PROJ_NAME_LC)
    unset(MAIN_INCLUDE_NAME)
    unset(MAIN_INCLUDE_PATH)
    unset(COMPONENT_INCLUDE_DIR)

    ]]
)

#--------------------Package Config----------------
configure_file(${CMAKE_PROJECT_NAME}-config.cmake
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake"
    COPYONLY
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config-version.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY ExactVersion
)

install(FILES
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake"
    DESTINATION cmake/${CMAKE_PROJECT_NAME}
)
