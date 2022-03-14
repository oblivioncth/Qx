#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
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
    qx-versionnumber.h
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
    qx-versionnumber.cpp
    qx-string.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qx::Utility
)

#=============== Component Handler =======================
include(component_lib_rules)
