#================= Project Setup ==========================
# CMake
cmake_minimum_required(VERSION 3.21.1)

# Project
project(Qx VERSION 0.1.0.0 LANGUAGES CXX)
set(PROJ_SHORT_NAME qx)
string(TOUPPER ${PROJ_SHORT_NAME} PROJ_SHORT_NAME_UC)

# Default Edtion
if(NOT EDITION)
    set(EDITION "widgets") # Default to widgets ("console" is other valid option)
endif()

# Set Edition letter
set(EDITION_LETTER $<IF:$<STREQUAL:${EDITION},widgets>,W,C>)

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LIB_)

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
set(SOURCE_DIR_NAME src)
set(INSTALL_DIR_NAME dist)
set(STATIC_LIB_INSTALL_DIR_NAME lib)
set(SHARED_LIB_INSTALL_DIR_NAME bin)
set(HEADER_INSTALL_DIR_NAME include)

# Paths
set(CMAKE_UTIL_DIR "${CMAKE_SOURCE_DIR}/${CMAKE_UTIL_DIR_NAME}")
set(CMAKE_MODULE_PATH "${CMAKE_UTIL_DIR}/${CMAKE_MODULE_DIR_NAME}")
set(CONFIG_FILE_DIR "${CMAKE_UTIL_DIR}/${CONFIG_FILE_DIR_NAME}")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/${SOURCE_DIR_NAME}")
set(MODULE_SOURCE_PREFIX "${SOURCE_DIR}/${PROJ_SHORT_NAME}")
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/${INSTALL_DIR_NAME}/${EDITION}/${CMAKE_BUILD_TYPE}")
set(HEADER_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${HEADER_INSTALL_DIR_NAME}")

# Includes
include_directories(${MODULE_SOURCE_PREFIX})

# Clean install when clean target is ran
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_INSTALL_PREFIX}")

################# Common Build #################

#--------- Edition Common -----------------------
set(LIB_MODULES
	core
	io
        utility
	network
	xml
)

# Qt package components
set(QT_FIND_COMPONENTS
    Core
    Network
    Xml
)

#--------- Edition Console -----------------------

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")
    set(QT_FIND_COMPONENTS ${QT_FIND_COMPONENTS}
            Widgets
            Gui
    )

    list(APPEND LIB_MODULES widgets)
endif()


################# Windows Build #################
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    list(APPEND LIB_MODULES windows)
endif()

################# Linux Build #################
if(CMAKE_SYSTEM_NAME STREQUAL Linux)

endif()

#================= Module Setup ==========================
# Find Qt package
find_package(Qt6 REQUIRED COMPONENTS ${QT_FIND_COMPONENTS})

# Build modules
foreach(module ${LIB_MODULES})
    add_subdirectory(${MODULE_SOURCE_PREFIX}/${module})
endforeach()

#================= Install ===============================

# Set variables in install/code context
install(CODE "set(CONFIG_FILE_DIR \"${CONFIG_FILE_DIR}\")")
install(CODE "set(LIB_NAME \"${CMAKE_PROJECT_NAME}\")")
install(CODE "set(LIB_NAME_UC \"${PROJ_SHORT_NAME_UC}\")")
install(CODE "set(LIB_EDITION \"${EDITION}\")")
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
install(CODE "set(PROJ_SHORT_NAME \"${PROJ_SHORT_NAME}\")")
install(CODE "set(MAIN_INCLUDE_NAME \"${PROJ_SHORT_NAME_UC}_H\")")
install(CODE "set(MAIN_INCLUDE_PATH \"${HEADER_INSTALL_PREFIX}/${PROJ_SHORT_NAME}.h\")")
install(CODE "set(MODULE_INCLUDE_DIR \"${HEADER_INSTALL_PREFIX}/${PROJ_SHORT_NAME}\")")

# Generate main public include header
install(CODE [[
    # Glob module include groups
    file(GLOB MODULE_INCLUDE_GROUPS
        RELATIVE "${MODULE_INCLUDE_DIR}"
        "${MODULE_INCLUDE_DIR}/*.h"
    )

    # Generate include statements
    foreach(modig ${MODULE_INCLUDE_GROUPS})
        set(MODULE_INCLUDE_FILES "${MODULE_INCLUDE_FILES}#include \"${PROJ_SHORT_NAME}/${modig}\"\n")
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
    unset(MODULE_INCLUDE_FILES)
    unset(BUILD_DATE)
    unset(CONFIG_FILE_DIR)
    unset(LIB_NAME)
    unset(LIB_NAME_UC)
    unset(LIB_EDITION)
    unset(LIB_VER)
    unset(LIB_VER_MAJOR)
    unset(LIB_VER_MINOR)
    unset(LIB_VER_REV)
    unset(LIB_VER_BUILD)
    unset(QT_VER)
    unset(PLATFORM)
    unset(BUILD_DATE)
    unset(PROJ_SHORT_NAME)
    unset(MAIN_INCLUDE_NAME)
    unset(MAIN_INCLUDE_PATH)
    unset(MODULE_INCLUDE_DIR)

    ]]
)