macro(find_qt6_package)
    # Find Qt normally
    find_package(Qt6 ${ARGN})

    # Determine install prefix
    if(PACKAGE_PREFIX_DIR) # Defined by Qt6Config.cmake
        set(Qt6_PREFIX_PATH ${PACKAGE_PREFIX_DIR})
    else()
        # Determine based on config script path
        cmake_path(REMOVE_FILENAME ${Qt6_CONFIG}
            OUTPUT_VARIABLE __QT_CONFIG_PATH
        )
        cmake_path(ABSOLUTE_PATH "${__QT_CONFIG_PATH}/../../../"
            NORMALIZE
            OUTPUT_VARIABLE Qt6_PREFIX_PATH
        )
    endif()

    # Ensure install prefix is valid
    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        set(__QMAKE_NAME "qmake.exe")
    elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
        set(__QMAKE_NAME "qmake")
    endif()

    if(NOT EXISTS "${Qt6_PREFIX_PATH}/bin/${__QMAKE_NAME}")
        message(FATAL_ERROR "Qt6_PREFIX_PATH could not be determined!")
    endif()

    # Determine Linkage
    get_target_property(__QT_CORE_TARGET_TYPE Qt6::Core TYPE)
    if(__QT_CORE_TARGET_TYPE STREQUAL SHARED_LIBRARY)
        set(Qt6_LINKAGE shared)
    elseif(__QT_CORE_TARGET_TYPE STREQUAL STATIC_LIBRARY)
        set(Qt6_LINKAGE static)
    else()
        message(FATAL_ERROR "Qt6 Core target type has an unexpected value!")
    endif()
endmacro()
