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

# Source Files
set(COMPONENT_IMPL
    qx-common-widgets.cpp
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
include(component_lib_rules)

