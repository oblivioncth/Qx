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
set(FILE_TEMPLATES_DIR_NAME file_templates)
set(COMPONENT_DIR_NAME comp)
set(OUT_DIR_NAME out)
set(STATIC_LIB_INSTALL_DIR_NAME lib)
set(SHARED_LIB_INSTALL_DIR_NAME bin)
set(HEADER_INSTALL_DIR_NAME include)

# Paths
set(CMAKE_UTIL_DIR "${CMAKE_SOURCE_DIR}/${CMAKE_UTIL_DIR_NAME}")
set(CMAKE_MODULE_PATH "${CMAKE_UTIL_DIR}/${CMAKE_MODULE_DIR_NAME}")
set(FILE_TEMPLATES_DIR "${CMAKE_UTIL_DIR}/${FILE_TEMPLATES_DIR_NAME}")
set(COMPONENTS_DIR "${CMAKE_SOURCE_DIR}/${COMPONENT_DIR_NAME}")
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/${OUT_DIR_NAME}/install")
set(PACKAGE_PREFIX "${CMAKE_SOURCE_DIR}/${OUT_DIR_NAME}/dist")
set(HEADER_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${HEADER_INSTALL_DIR_NAME}")

# Clean install when clean target is ran
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_INSTALL_PREFIX}")

# General Variables
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TARGET_ARCH x64)
else()
  set(TARGET_ARCH x86)
endif()

################# Common Build #################
set(LIB_COMPONENTS
    core
    gui
    io
    utility
    network
    widgets
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

#================= Top Level Build =========================

# Find Qt package
find_package(Qt6 REQUIRED COMPONENTS ${QT_FIND_COMPONENTS})

#--------------------Package Config----------------

# Create config file
configure_file(${CMAKE_PROJECT_NAME}Config.cmake.in
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
    @ONLY
)

# Create version config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY ExactVersion
)

#================= Process Componets =======================

# Build components
foreach(component ${LIB_COMPONENTS})
    add_subdirectory(${COMPONENTS_DIR}/${component})
endforeach()

#================= Top Level Install =======================

# Install Package Config
install(FILES
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION lib/cmake/${CMAKE_PROJECT_NAME}
)

# Install README and LICENSE
install(FILES
    "${CMAKE_SOURCE_DIR}/README.md"
    "${CMAKE_SOURCE_DIR}/LICENSE"
    DESTINATION .
)

#====================== CPack ==============================

set(CPACK_PACKAGE_VENDOR "oblivioncth")
set(CPACK_PACKAGE_DIRECTORY "${PACKAGE_PREFIX}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${PROJECT_VERSION}_(Qt${Qt6_VERSION})_${CMAKE_SYSTEM_NAME}_${TARGET_ARCH}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
include(CPack)
