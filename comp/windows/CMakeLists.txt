#================= Setup Component ==========================
set(COMPONENT_NAME "Windows")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    LIB_TYPE "STATIC"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PUBLIC
            qx-common-windows.h
            qx-filedetails.h
            qx-windefs.h
    IMPLEMENTATION
        qx-common-windows.cpp
        qx-filedetails.cpp
    DOC_ONLY
        qx-windefs.dox
    LINKS
        PRIVATE
            Qx::Io
        PUBLIC
            Qt6::Core
            Version.lib
            Qx::Core
)
