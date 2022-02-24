#================= Setup Module ==========================

set(MODULE_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(MODULE_PUBLIC_HEADERS
    qx-common-network.h
    qx-networkreplyerror.h
    qx-syncdownloadmanager.h
)

# Source Files
set(MODULE_IMPL
    qx-common-network.cpp
    qx-networkreplyerror.cpp
    qx-syncdownloadmanager.cpp
)

# Links
set(MODULE_PUBLIC_LINKS
    Qt6::Core
    Qt6::Network
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()

#=============== Module Handler =======================
include(lib_module_rules)

