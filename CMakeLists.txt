#================= Project Setup ==========================

# CMake
cmake_minimum_required(VERSION 3.23.0...3.26.0)

# Project
# NOTE: For versions stick to w.x.y.z, where z is generally
# avoided and only used for hotfixes. DON'T USE TRAILING
# ZEROS IN VERSIONS
project(Qx
    VERSION 0.5.1
    LANGUAGES CXX
    DESCRIPTION "Qt Extensions Library"
)

# Get helper scripts
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchOBCMake.cmake)
fetch_ob_cmake("v0.3.2")

# Initialize project according to standard rules
include(OB/Project)
ob_standard_project_setup()

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build augmentation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#--------------------Setup Paths-------------------

# Component libraries
set(COMPONENTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")

#------------Enumerate Components----------------
include(OB/Utility)

ob_get_subdirectory_list(${COMPONENTS_PATH} LIB_COMPONENTS)
foreach(comp ${LIB_COMPONENTS})
    ob_string_to_proper_case("${comp}" COMP_PC)
    list(APPEND AVAILABLE_COMPONENTS "${COMP_PC}")
endforeach()

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    list(REMOVE_ITEM AVAILABLE_COMPONENTS "Linux")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    list(REMOVE_ITEM AVAILABLE_COMPONENTS "Windows")
    list(REMOVE_ITEM AVAILABLE_COMPONENTS "Windows-gui")
endif()

message(STATUS "Available Qx components: ${AVAILABLE_COMPONENTS}")

#------------Setup user options----------------

# Configuration options
set(QX_COMPONENTS "${AVAILABLE_COMPONENTS}" CACHE STRING "Qx components to configure")
option(QX_DOCS "Build Qx documentation." OFF)
option(QX_TESTS "Build the Qx tests." OFF)
option(BUILD_SHARED_LIBS "Build shared libraries." OFF) # Redundant due to OB, but explicit

# Confirm user component choices are valid and normalize casing
if(NOT QX_COMPONENTS)
    message(FATAL_ERROR "QX_COMPONENTS must contain at least one component!")
endif()
foreach(sel_comp ${QX_COMPONENTS})
    ob_string_to_proper_case("${sel_comp}" SEL_COMP_PC)
    if(NOT SEL_COMP_PC IN_LIST AVAILABLE_COMPONENTS)
        message(FATAL_ERROR "${SEL_COMP_PC} is not an available ${PROJECT_NAME} component!")
    endif()

    list(APPEND QX_COMPONENTS_NORMALIZED "${SEL_COMP_PC}")
endforeach()


message(STATUS "Selected Qx components: ${QX_COMPONENTS_NORMALIZED}")

#================= Top Level Build =========================

# Determine full component list
include(Qx/ComponentHelper)
qx_enumerate_sibling_tree("${QX_COMPONENTS_NORMALIZED}" QX_COMPONENTS_FULL)
message(STATUS "Configured Qx components: ${QX_COMPONENTS_FULL}")

# Determine all Qt dependencies
qx_get_all_qt_depends("${QX_COMPONENTS_FULL}" ALL_QT_COMPONENTS)
message(STATUS "All Qx Qt component dependencies: ${ALL_QT_COMPONENTS}")

if(QX_TESTS)
    list(APPEND ALL_QT_COMPONENTS Test)
endif()

# Find Qt package
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)
include(OB/BetterFindQt6)
ob_find_qt6_package(REQUIRED COMPONENTS ${ALL_QT_COMPONENTS})

#--------------------Package Config-----------------------

ob_standard_project_package_config(
    COMPATIBILITY "SameMinorVersion"
    CONFIG CUSTOM "${PROJECT_FILE_TEMPLATES}/${PROJECT_NAMESPACE}Config.cmake.in"
)

#================= Process Componets =======================

# Configure requested components
foreach(component ${QX_COMPONENTS_FULL})
    string(TOLOWER ${component} component_lc)
    add_subdirectory(${COMPONENTS_PATH}/${component_lc})
endforeach()

#================ Build Documentation ======================

if(QX_DOCS)
    set(DOC_TARGET_NAME ${PROJECT_NAMESPACE_LC}_docs)
    add_subdirectory(doc)
endif()

#================ Build Tests ======================
if(QX_TESTS)
    enable_testing()
    set(TESTS_TARGET_PREFIX ${PROJECT_NAMESPACE_LC})
    set(TESTS_COMMON_TARGET "${TESTS_TARGET_PREFIX}_tst_common")
    add_subdirectory(tests)
endif()

#================= Top Level Install =======================

ob_standard_project_misc_install()

#====================== Package ==============================

include(OB/Packaging)
ob_standard_project_package(VENDOR "oblivioncth")
