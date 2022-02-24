#================= Setup Module ==========================

set(MODULE_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(MODULE_PUBLIC_HEADERS
    qx-common-windows.h
    qx-filedetails.h
)

# Source Files
set(MODULE_IMPL
    qx-common-windows.cpp
    qx-filedetails.cpp
)

# Links
set(MODULE_PUBLIC_LINKS
    Qt::Core
    Version.lib
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()

#=============== Module Handler =======================
include(lib_module_rules)

