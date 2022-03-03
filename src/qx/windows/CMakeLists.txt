#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(COMPONENT_PUBLIC_API_HEADERS
    qx-common-windows.h
    qx-filedetails.h
)

# Source Files
set(COMPONENT_IMPL
    qx-common-windows.cpp
    qx-filedetails.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt::Core
    Version.lib
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()

#=============== Component Handler =======================
include(component_lib_rules)

