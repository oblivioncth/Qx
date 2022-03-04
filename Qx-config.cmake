include(CMakeFindDependencyMacro)
find_dependency(Qt6 6.2)

file(GLOB AVAILABLE_COMPONENT_CONFIGS
	RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
	"${CMAKE_CURRENT_LIST_DIR}/Qx-*-config.cmake"
)

foreach(component ${Qx_FIND_COMPONENTS})
	set(component_config Qx-${component}-config.cmake)

	if (";${AVAILABLE_COMPONENT_CONFIGS};" MATCHES ";${component_config};")
		include("${CMAKE_CURRENT_LIST_DIR}/${component_config}")
	elseif(Qx_FIND_REQUIRED_${component}})
		set(Qx_FOUND False)
		set(Qx_NOT_FOUND_MESSAGE "Unsupported component: ${component}")
	endif()
endforeach()