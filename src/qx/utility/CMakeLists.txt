#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE INTERFACE)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(COMPONENT_PUBLIC_API_HEADERS
    qx-concepts.h
    qx-helpers.h
    qx-macros.h
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()

#=============== Component Handler =======================
include(component_lib_rules)
