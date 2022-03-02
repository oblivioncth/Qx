#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(COMPONENT_PUBLIC_HEADERS
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
    qx-traverser.h
)

# Source Files
set(COMPONENT_IMPL
    qx-bitarray.cpp
    qx-char.cpp
    qx-datetime.cpp
    qx-genericerror.cpp
    qx-integrity.cpp
    qx-json.cpp
    qx-list.cpp
    qx-mmrb.cpp
    qx-string.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt::Core
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

    # Headers
    set(COMPONENT_PUBLIC_HEADERS ${COMPONENT_PUBLIC_HEADERS}
        qx-color.h
    )

    # Source Files
    set(COMPONENT_IMPL ${COMPONENT_IMPL}
        qx-color.cpp
    )

    # Links
    set(COMPONENT_PUBLIC_LINKS ${COMPONENT_PUBLIC_LINKS}
        Qt6::Widgets
        Qt6::Gui
    )
endif()

#=============== Component Handler =======================
include(component_lib_rules)
