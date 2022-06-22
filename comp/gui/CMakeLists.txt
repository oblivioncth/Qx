#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-color.h
)

# Source Files
set(COMPONENT_IMPL
    qx-color.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qt6::Gui
)

#=============== Component Handler =======================
include(Qx/ComponentHelper)
register_qx_component()

