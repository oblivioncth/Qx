#================= Setup Module ==========================

set(MODULE_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(MODULE_PUBLIC_HEADERS
    qx-logindialog.h
    qx-standarditemmodel.h
    qx-treeinputdialog.h
)

# Source Files
set(MODULE_IMPL
    qx-logindialog.cpp
    qx-standarditemmodel.cpp
    qx-treeinputdialog.cpp
)

# Links
set(MODULE_PUBLIC_LINKS
    Qt::Core
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")
    # Links
    set(MODULE_PUBLIC_LINKS ${MODULE_PUBLIC_LINKS}
        Qt6::Widgets
        Qt6::Gui
    )
endif()

#=============== Module Handler =======================
include(lib_module_rules)

