#================= Setup ==========================
cmake_minimum_required(VERSION 3.21.1)

project(Qx VERSION 1.0.0 LANGUAGES CXX)

if(NOT EDITION)
    set(EDITION "widgets") # Default to widgets ("console" is other valid option)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)

set(LIB_MODULE_PREFIX src/qx)

include_directories(${LIB_MODULE_PREFIX})

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
if(WIN32)
	list(APPEND LIB_MODULES windows)
endif()

################# Linux Build #################
if(UNIX AND NOT APPLE)

endif()

#================= Build ==========================
# Find Qt package
find_package(Qt6 REQUIRED COMPONENTS ${QT_FIND_COMPONENTS})

foreach(module ${LIB_MODULES})
    add_subdirectory(${LIB_MODULE_PREFIX}/${module})
endforeach()


