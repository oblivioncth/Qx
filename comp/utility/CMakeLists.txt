#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE INTERFACE)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-concepts.h
    qx-helpers.h
    qx-macros.h
)

# Doc Only Files
set(COMPONENT_DOC_ONLY
    qx-concepts.dox
    qx-helpers.dox
    qx-macros.dox
)

#=============== Component Handler =======================
include(ComponentHelper)
register_qx_component()
