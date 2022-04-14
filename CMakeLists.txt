#================= Project Setup ==========================
# CMake
cmake_minimum_required(VERSION 3.21.1)

# Project
project(Qx
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Qt Extensions Library"
)
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

# General Variables
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TARGET_ARCH x64)
else()
  set(TARGET_ARCH x86)
endif()


#--------------------Setup Paths-------------------

# Cmake scripts
set(PROJ_SCRIPTS_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${PROJ_SCRIPTS_PATH}/module")
set(FILE_TEMPLATES_PATH "${PROJ_SCRIPTS_PATH}/file_templates")
set(DOC_SCRIPTS_PATH "${PROJ_SCRIPTS_PATH}/doc")

# Source
set(DOC_SOURCE_PATH "${CMAKE_SOURCE_DIR}/doc")

# Sub projects
set(COMPONENTS_PATH "${CMAKE_SOURCE_DIR}/comp")

# Build
set(DOC_BUILD_PATH "${CMAKE_BINARY_DIR}/doc")

# Install
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out/install")
set(HEADER_INSTALL_SUFFIX "include")

# Package
set(PACKAGE_PREFIX "${CMAKE_SOURCE_DIR}/out/dist")

#------------Set Global Build Rules----------------

# Clean install when clean target is ran
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_INSTALL_PREFIX}")

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

# Find Doxygen package
find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia
)

#--------------------Package Config-----------------------

# Create config file
configure_file("${FILE_TEMPLATES_PATH}/${CMAKE_PROJECT_NAME}Config.cmake.in"
    "${CMAKE_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake"
    @ONLY
)

# Create version config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

#================= Process Componets =======================

# Build components
foreach(component ${LIB_COMPONENTS})
    add_subdirectory(${COMPONENTS_PATH}/${component})
endforeach()

#================ Build Documentation ======================

if(DOXYGEN_FOUND)
    # Configure files
    configure_file("${FILE_TEMPLATES_PATH}/doc_mainpage.md.in"
        "${CMAKE_BINARY_DIR}/docin/mainpage.md"
        @ONLY
    )

    # Top level input
    set(DOC_INPUT_LIST
        "${CMAKE_BINARY_DIR}/docin/mainpage.md"
        "${DOC_SOURCE_PATH}/namespace.dox"
    )

    # Component level input
    foreach(component ${LIB_COMPONENTS})
        set(DOC_INPUT_LIST ${DOC_INPUT_LIST}
            "${COMPONENTS_PATH}/${component}/doc/${component}.dox"
            "${COMPONENTS_PATH}/${component}/src"
            "${COMPONENTS_PATH}/${component}/include"
        )

        if(EXISTS "${COMPONENTS_PATH}/${component}/doc/snippets")
            set(DOC_EXAMPLE_LIST ${DOC_EXAMPLE_LIST}
                "${COMPONENTS_PATH}/${component}/doc/snippets"
            )
        endif()

        if(EXISTS "${COMPONENTS_PATH}/${component}/doc/images")
            set(DOC_IMAGE_LIST ${DOC_IMAGE_LIST}
                "${COMPONENTS_PATH}/${component}/doc/images"
            )
        endif()
    endforeach()

    # Set Doxygen parameters
    include(${DOC_SCRIPTS_PATH}/doxyconf.cmake)

    # Add Doxygen target
    doxygen_add_docs(docs
        ${DOC_INPUT_LIST}
    )

    message(STATUS "Doxygen configured. Build target 'docs' to build the documentation.")
else()
    message(WARNING "Doxygen not available, documentation cannot not be built.")
endif()

#================= Top Level Install =======================

# Install Package Config
install(FILES
    "${CMAKE_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake"
    "${CMAKE_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION cmake
)

# Install README and LICENSE
install(FILES
    "${CMAKE_SOURCE_DIR}/README.md"
    "${CMAKE_SOURCE_DIR}/LICENSE"
    DESTINATION .
)

# Install Docs if available
install(DIRECTORY ${DOC_BUILD_PATH} DESTINATION . OPTIONAL)

#====================== CPack ==============================

set(CPACK_PACKAGE_VENDOR "oblivioncth")
set(CPACK_PACKAGE_DIRECTORY "${PACKAGE_PREFIX}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${PROJECT_VERSION}_(Qt${Qt6_VERSION})_${CMAKE_SYSTEM_NAME}_${TARGET_ARCH}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
include(CPack)
