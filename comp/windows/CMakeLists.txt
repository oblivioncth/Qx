#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-common-windows.h
    qx-filedetails.h
    qx-windefs.h
)

# Source Files
set(COMPONENT_IMPL
    qx-common-windows.cpp
    qx-filedetails.cpp
)

# Doc Only Files
set(COMPONENT_DOC_ONLY
    qx-windefs.dox
)

# Links
set(COMPONENT_PRIVATE_LINKS
    Qx::Io
)

set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Version.lib
    Qx::Core
)

# Inter-project Dependencies
set(COMPONENT_SIBLING_DEPENDS
    Core
    Io
)

# Qt Component Dependencies
set(COMPONENT_QT_DEPENDS
    Core
)

#=============== Component Handler =======================
include(Qx/ComponentHelper)
register_qx_component()

