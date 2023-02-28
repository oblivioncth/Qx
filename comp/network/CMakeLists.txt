#================= Setup Component ==========================
set(COMPONENT_NAME "Network")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PUBLIC
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
