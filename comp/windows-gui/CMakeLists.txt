#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-taskbarbutton.h
)

# Source Files
set(COMPONENT_IMPL
    qx-taskbarbutton.cpp
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

# Inter-project Dependencies
set(COMPONENT_SIBLING_DEPENDS
    Core
    Windows
)

# Qt Component Dependencies
set(COMPONENT_QT_DEPENDS
    Core
    Gui
)

#=============== Component Handler =======================
include(component_lib_rules)

