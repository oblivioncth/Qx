#================= Setup Module ==========================

set(MODULE_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(MODULE_PUBLIC_HEADERS
    qx-common-xml.h
    qx-xmlstreamreadererror.h
    qx-xmlstreamwritererror.h
)

# Source Files
set(MODULE_IMPL
    qx-common-xml.cpp
    qx-xmlstreamreadererror.cpp
    qx-xmlstreamwritererror.cpp
)

# Links
set(MODULE_PUBLIC_LINKS
    Qt6::Core
    Qt6::Xml
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()

#=============== Module Handler =======================
include(lib_module_rules)

