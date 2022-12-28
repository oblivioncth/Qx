#================= Setup Component ==========================
set(COMPONENT_NAME "Gui")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    LIB_TYPE "STATIC"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PUBLIC
            qx-color.h
    IMPLEMENTATION
        qx-color.cpp
    LINKS
        PUBLIC
            Qt6::Core
            Qt6::Gui
)

