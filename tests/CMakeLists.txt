add_subdirectory(_common)

# Add tests for each configured component
foreach(comp ${QX_COMPONENTS_FULL})
    string(TOLOWER ${comp} comp_lc)
    add_subdirectory(${comp_lc})
endforeach()
