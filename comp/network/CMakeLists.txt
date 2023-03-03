#================= Add Component ==========================
qx_add_component("Network"
    HEADERS_API
        qx-common-network.h
        qx-networkreplyerror.h
        qx-downloadmanager.h
    IMPLEMENTATION
        qx-common-network.cpp
        qx-networkreplyerror.cpp
        qx-downloadmanager.cpp
    LINKS
        PUBLIC
            Qt6::Core
            Qt6::Network
            Qx::Core
            Qx::Io
)
