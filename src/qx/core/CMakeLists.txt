#================= Setup Module ==========================

set(MODULE_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(MODULE_PUBLIC_HEADERS
    qx-algorithm.h
    qx-array.h
    qx-bitarray.h
    qx-bytearray.h
    qx-char.h
    qx-cumulation.h
    qx-datetime.h
    qx-endian.h
    qx-freeindextracker.h
    qx-genericerror.h
    qx-index.h
    qx-integrity.h
    qx-iostream.h
    qx-json.h
    qx-list.h
    qx-mmrb.h
    qx-regularexpression.h
    qx-string.h
    qx-stringtraverser.h
)

# Source Files
set(MODULE_IMPL
    qx-bitarray.cpp
    qx-char.cpp
    qx-datetime.cpp
    qx-genericerror.cpp
    qx-integrity.cpp
    qx-json.cpp
    qx-list.cpp
    qx-mmrb.cpp
    qx-string.cpp
    qx-stringtraverser.cpp
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

    # Headers
    set(MODULE_PUBLIC_HEADERS ${MODULE_PUBLIC_HEADERS}
        qx-color.h
    )

    # Source Files
    set(MODULE_IMPL ${MODULE_IMPL}
        qx-color.cpp
    )

    # Links
    set(MODULE_PUBLIC_LINKS ${MODULE_PUBLIC_LINKS}
        Qt6::Widgets
        Qt6::Gui
    )
endif()

#=============== Module Handler =======================
include(lib_module_rules)
