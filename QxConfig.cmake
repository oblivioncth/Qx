include(CMakeFindDependencyMacro)
find_dependency(Qt6 6.2)

file(GLOB __QX_PKG_CMAKE_FILES
    RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/*.cmake"
)

list(FILTER __QX_PKG_CMAKE_FILES __QX_AVAILABLE_COMPONENT_TARGETS
    INCLUDE
    REGEX "/Qx[A-Z]\w+Targets\.cmake"
)

foreach(component ${Qx_FIND_COMPONENTS})
        set(component_target Qx${component}Targets.cmake)

        if (";${__QX_AVAILABLE_COMPONENT_TARGETS};" MATCHES ";${component_target};")
                include("${CMAKE_CURRENT_LIST_DIR}/${component_target}")
	elseif(Qx_FIND_REQUIRED_${component}})
		set(Qx_FOUND False)
		set(Qx_NOT_FOUND_MESSAGE "Unsupported component: ${component}")
	endif()
endforeach()
