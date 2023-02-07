#================= Setup Component ==========================
set(COMPONENT_NAME "Xml")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    LIB_TYPE "STATIC"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PUBLIC
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
