#================= Setup Module ==========================

set(MODULE_TYPE INTERFACE)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(MODULE_PUBLIC_HEADERS
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

#=============== Module Handler =======================
include(lib_module_rules)
