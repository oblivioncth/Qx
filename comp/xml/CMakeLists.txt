#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-common-xml.h
    qx-xmlstreamreadererror.h
    qx-xmlstreamwritererror.h
)

# Source Files
set(COMPONENT_IMPL
    qx-common-xml.cpp
    qx-xmlstreamreadererror.cpp
    qx-xmlstreamwritererror.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qt6::Xml
)

#=============== Component Handler =======================
include(component_lib_rules)

