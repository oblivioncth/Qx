#================= Setup Component ==========================
set(COMPONENT_NAME "Windows-gui")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    LIB_TYPE "STATIC"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PRIVATE
            qx-winguieventfilter_p.h
        PUBLIC
            qx-taskbarbutton.h
            qx-winguievent.h
    IMPLEMENTATION
        qx-taskbarbutton.cpp
        qx-winguievent.cpp
        qx-winguieventfilter.cpp
    LINKS
        PRIVATE
            Qx::Windows
            Qx::Core
        PUBLIC
            Qt6::Core
            Qt6::Gui
)
