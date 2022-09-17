#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-desktopentry.h
    qx-applicationdesktopentry.h
    qx-linkdesktopentry.h
    qx-directorydesktopentry.h
)

# Source Files
set(COMPONENT_IMPL
    qx-desktopentry.cpp
    qx-applicationdesktopentry.cpp
    qx-linkdesktopentry.cpp
    qx-directorydesktopentry.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qx::Core
    Qx::Io
)

#=============== Component Handler =======================
include(Qx/ComponentHelper)
register_qx_component()

