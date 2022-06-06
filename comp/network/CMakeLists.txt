#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-common-network.h
    qx-networkreplyerror.h
    qx-downloadmanager.h
)

# Source Files
set(COMPONENT_IMPL
    qx-common-network.cpp
    qx-networkreplyerror.cpp
    qx-downloadmanager.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qt6::Network
    Qx::Core
    Qx::Io
)

# Inter-project Dependencies
set(COMPONENT_SIBLING_DEPENDS
    Core
    Io
)

# Qt Component Dependencies
set(COMPONENT_QT_DEPENDS
    Core
    Network
)

#=============== Component Handler =======================
include(Qx/ComponentHelper)
register_qx_component()

