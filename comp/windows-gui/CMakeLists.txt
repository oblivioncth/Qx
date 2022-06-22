#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-taskbarbutton.h
    qx-winguievent.h
)

set(COMPONENT_PRIVATE_HEADERS
    qx-winguieventfilter_p.h
)

# Source Files
set(COMPONENT_IMPL
    qx-taskbarbutton.cpp
    qx-winguievent.cpp
    qx-winguieventfilter.cpp
)

# Links
set(COMPONENT_PRIVATE_LINKS
    Qx::Windows
    Qx::Core
)

set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qt6::Gui
)

#=============== Component Handler =======================
include(Qx/ComponentHelper)
register_qx_component()

