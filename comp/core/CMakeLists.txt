#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-algorithm.h
    qx-array.h
    qx-base85.h
    qx-bitarray.h
    qx-bytearray.h
    qx-char.h
    qx-cumulation.h
    qx-datetime.h
    qx-dsvtable.h
    qx-freeindextracker.h
    qx-genericerror.h
    qx-groupedprogressmanager.h
    qx-index.h
    qx-integrity.h
    qx-iostream.h
    qx-json.h
    qx-list.h
    qx-progressgroup.h
    qx-table.h
    qx-versionnumber.h
    qx-regularexpression.h
    qx-setonce.h
    qx-string.h
    qx-system.h
    qx-traverser.h
)
set(COMPONENT_PRIVATE_HEADERS
    qx-json_p.h
)

# Source Files
set(COMPONENT_IMPL
    qx-algorithm.cpp
    qx-base85.cpp
    qx-bitarray.cpp
    qx-char.cpp
    qx-datetime.cpp
    qx-dsvtable.cpp
    qx-genericerror.cpp
    qx-groupedprogressmanager.cpp
    qx-integrity.cpp
    qx-json.cpp
    qx-json_p.cpp
    qx-progressgroup.cpp
    qx-versionnumber.cpp
    qx-string.cpp
    qx-system.cpp
)

# Doc Only Files
set(COMPONENT_DOC_ONLY
    qx-freeindextracker.dox
    qx-regularexpression.dox
    qx-bytearray.dox
    qx-index.dox
    qx-iostream.dox
    qx-list.dox
    qx-traverser.dox
    qx-cumulation.dox
    qx-array.dox
    qx-setonce.dox
    qx-table.dox
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qx::Utility
)

################# Windows Build #################
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    list(APPEND COMPONENT_IMPL
        qx-iostream_win.cpp
        qx-system_win.cpp
    )
endif()


################# Linux Build #################
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    list(APPEND COMPONENT_IMPL
        qx-iostream_linux.cpp
        qx-system_linux.cpp
    )
endif()

#=============== Component Handler =======================
include(Qx/ComponentHelper)
register_qx_component()
