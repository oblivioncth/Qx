#================= Setup Component ==========================
set(COMPONENT_NAME "Utility")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    LIB_TYPE "INTERFACE"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PUBLIC
            qx-concepts.h
            qx-helpers.h
            qx-macros.h
    DOC_ONLY
        qx-concepts.dox
        qx-helpers.dox
        qx-macros.dox
)
