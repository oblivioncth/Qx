#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-common-widgets.h
    qx-logindialog.h
    qx-standarditemmodel.h
    qx-treeinputdialog.h
)

set(COMPONENT_PRIVATE_HEADERS
    qx-common-widgets_p.h
)

# Source Files
set(COMPONENT_IMPL
    qx-common-widgets.cpp
    qx-common-widgets_p.cpp
    qx-logindialog.cpp
    qx-standarditemmodel.cpp
    qx-treeinputdialog.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qt6::Widgets
    Qx::Core
)

#=============== Component Handler =======================
include(Qx/ComponentHelper)
register_qx_component()

