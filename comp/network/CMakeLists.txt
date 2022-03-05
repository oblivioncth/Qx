#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(COMPONENT_PUBLIC_API_HEADERS
    qx-common-network.h
    qx-networkreplyerror.h
    qx-syncdownloadmanager.h
)

# Source Files
set(COMPONENT_IMPL
    qx-common-network.cpp
    qx-networkreplyerror.cpp
    qx-syncdownloadmanager.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qt6::Network
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()

#=============== Component Handler =======================
include(component_lib_rules)

