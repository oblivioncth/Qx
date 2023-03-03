#================= Add Component ==========================
qx_add_component("Xml"
    HEADERS_API
        qx-common-xml.h
        qx-xmlstreamreadererror.h
        qx-xmlstreamwritererror.h
    IMPLEMENTATION
        qx-common-xml.cpp
        qx-xmlstreamreadererror.cpp
        qx-xmlstreamwritererror.cpp
    LINKS
        PUBLIC
            Qt6::Core
            Qt6::Xml
)
