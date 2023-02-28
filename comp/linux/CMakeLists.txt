#================= Setup Component ==========================
set(COMPONENT_NAME "Linux")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PUBLIC
            qx-desktopentry.h
            qx-applicationdesktopentry.h
            qx-linkdesktopentry.h
            qx-directorydesktopentry.h
    IMPLEMENTATION
        qx-desktopentry.cpp
        qx-applicationdesktopentry.cpp
        qx-linkdesktopentry.cpp
        qx-directorydesktopentry.cpp
    LINKS
        PUBLIC
            Qt6::Core
            Qx::Core
            Qx::Io
)
