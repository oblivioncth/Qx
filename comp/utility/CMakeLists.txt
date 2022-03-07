#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE INTERFACE)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-concepts.h
    qx-helpers.h
    qx-macros.h
)

#=============== Component Handler =======================
include(component_lib_rules)
