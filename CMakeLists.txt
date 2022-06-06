#================= Project Setup ==========================

# CMake
cmake_minimum_required(VERSION 3.21.1)
if(NOT ${GENERATOR_IS_MULTI_CONFIG})
    message(FATAL_ERROR "This project currently only supports multi-config generators!")
endif()

# Store minimum required version for later since it can be overwritten
# by find_package/macro calls in dependencies (stupid)
set(TRUE_CMAKE_MINIMUM_REQUIRED ${CMAKE_MINIMUM_REQUIRED_VERSION})

# Configuration options
option(QX_DOCS_TARGET "Build Qx documentation" OFF)

# Project
# NOTE: For versions stick to w.x.y.z, where z is generally
# avoided and only used for hotfixes. DON'T USE TRAILING
# ZEROS IN VERSIONS
set(QX_BASE_VERSION 0.2) # Required for CI/CD
project(Qx
    VERSION ${QX_BASE_VERSION}
    LANGUAGES CXX
    DESCRIPTION "Qt Extensions Library"
)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LC)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UC)



# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build augmentation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Disable deprecated code
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Add local modules
set(PROJ_SCRIPTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${PROJ_SCRIPTS_PATH}/module")
include(Qx/Utility)

# General Variables
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TARGET_ARCH x64)
else()
  set(TARGET_ARCH x86)
endif()
get_proper_system_name(PROPER_SYSTEM_NAME)

#--------------Top Level Project Setup-------------

if(${PROJECT_IS_TOP_LEVEL})
    message(STATUS "NOTE: ${PROJECT_NAME} is being configured as a top-level project")

    # Install (override the CMake default, but not a user set value)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/out/install"
               CACHE PATH "Project install path" FORCE
        )
    endif()

    # Clean install when clean target is ran
    set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_INSTALL_PREFIX}")
else()
    message(STATUS "NOTE: ${PROJECT_NAME} is being configured as a sub-project")

    # Keep install components out of 'all' target
    set(SUB_PROJ_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
endif()

#--------------------Setup Paths-------------------

# Cmake scripts
set(FILE_TEMPLATES_PATH "${PROJ_SCRIPTS_PATH}/file_templates")
set(DOC_SCRIPTS_PATH "${PROJ_SCRIPTS_PATH}/doc")

# Source
set(DOC_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/doc")

# Sub projects
set(COMPONENTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/comp")

# Build
set(DOC_BUILD_PATH "${CMAKE_CURRENT_BINARY_DIR}/doc")
set(DOC_GENERATED_PATH "${CMAKE_CURRENT_BINARY_DIR}/docin")


set(HEADER_INSTALL_SUFFIX "include")

# Package
set(PACKAGE_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/out/dist")

#------------Set Global Build Rules----------------

# Versioning
include(Qx/VerboseVersioning)
setup_verbose_versioning(PROJECT_VERSION_VERBOSE)

################# Common Build #################

# Enumerate Components
get_subdirectory_list(${COMPONENTS_PATH} LIB_COMPONENTS)
set(BUILD_COMPONENTS ${LIB_COMPONENTS})

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

endif()

################# Linux Build #################
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    list(REMOVE_ITEM BUILD_COMPONENTS windows)
    list(REMOVE_ITEM BUILD_COMPONENTS windows-gui)
endif()

#================= Top Level Build =========================

# Find Qt package
include(Qx/BetterFindQt6)
find_qt6_package(REQUIRED COMPONENTS ${QT_FIND_COMPONENTS})

#--------------------Package Config-----------------------

# Create config file
configure_file("${FILE_TEMPLATES_PATH}/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
    @ONLY
)

# Create version config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

#================= Process Componets =======================

# Build components
foreach(component ${BUILD_COMPONENTS})
    add_subdirectory(${COMPONENTS_PATH}/${component})
endforeach()

#================ Build Documentation ======================

if(QX_DOCS_TARGET)
    # Find Doxygen package
    find_package(Doxygen
                 COMPONENTS dot
                 OPTIONAL_COMPONENTS mscgen dia
    )

    # Require Doxygen
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen not available, documentation cannot not be built!")
    endif()

    # Configure files
    configure_file("${FILE_TEMPLATES_PATH}/doc_mainpage.md.in"
        "${DOC_GENERATED_PATH}/mainpage.md"
        @ONLY
    )

    # Top level input
    set(DOC_INPUT_LIST
        "${DOC_GENERATED_PATH}/mainpage.md"
        "${DOC_SOURCE_PATH}/namespace.dox"
    )

    # Component level input
    foreach(component ${LIB_COMPONENTS})
        set(DOC_INPUT_LIST ${DOC_INPUT_LIST}
            "${COMPONENTS_PATH}/${component}/doc/${component}.dox"
            "${COMPONENTS_PATH}/${component}/src"
            "${COMPONENTS_PATH}/${component}/include"
        )

        if(EXISTS "${COMPONENTS_PATH}/${component}/doc/general")
            set(DOC_INPUT_LIST ${DOC_INPUT_LIST}
                "${COMPONENTS_PATH}/${component}/doc/general"
            )
        endif()

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

    # Determine Qt related doc paths
    include(Qx/QtDocsHelper)
    configure_qt_doc_link(${Qt6_PREFIX_PATH})

    # Set Doxygen parameters
    include(${DOC_SCRIPTS_PATH}/doxyconf.cmake)

    # Add Doxygen target
    doxygen_add_docs(docs
        ${DOC_INPUT_LIST}
    )

    message(STATUS "Doxygen configured. Build target 'docs' to build the documentation.")
endif()

#================= Top Level Install =======================

set(TOP_LEVEL_INSTALL_COMPONENT ${PROJECT_NAME_LC})

# Install Package Config
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
    COMPONENT ${TOP_LEVEL_INSTALL_COMPONENT}
    DESTINATION cmake
    ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
)

# Install README and LICENSE
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    COMPONENT ${TOP_LEVEL_INSTALL_COMPONENT}
    DESTINATION .
    ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
)

# Install Docs if available
install(DIRECTORY ${DOC_BUILD_PATH}
    COMPONENT ${PROJECT_NAME_LC}_docs
    DESTINATION .
    CONFIGURATIONS Release
    OPTIONAL
    ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
)

#====================== CPack ==============================

set(CPACK_PACKAGE_VENDOR "oblivioncth")
set(CPACK_PACKAGE_DIRECTORY "${PACKAGE_PREFIX}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${PROJECT_VERSION_VERBOSE}_(Qt${Qt6_VERSION}-${Qt6_LINKAGE})_${PROPER_SYSTEM_NAME}_${TARGET_ARCH}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
include(CPack)
