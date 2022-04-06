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

# Doc Only Files
set(COMPONENT_DOC_ONLY
    qx-freeindextracker.dox
    qx-regularexpression.dox
    qx-bytearray.dox
    qx-index.dox
    qx-algorithm.dox
    qx-iostream.dox
    qx-list.dox
    qx-traverser.dox
    qx-cumulation.dox
    qx-array.dox
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qx::Utility
)

# Inter-project Dependencies
set(COMPONENT_SIBLING_DEPENDS
    Utility
)

# Qt Component Dependencies
set(COMPONENT_QT_DEPENDS
    Core
)

#=============== Component Handler =======================
include(component_lib_rules)
