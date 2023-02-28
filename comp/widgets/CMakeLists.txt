#================= Setup Component ==========================
set(COMPONENT_NAME "Widgets")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PRIVATE
            qx-common-widgets_p.h
        PUBLIC
            qx-common-widgets.h
            qx-logindialog.h
            qx-standarditemmodel.h
            qx-treeinputdialog.h
    IMPLEMENTATION
        qx-common-widgets.cpp
        qx-common-widgets_p.cpp
        qx-logindialog.cpp
        qx-standarditemmodel.cpp
        qx-treeinputdialog.cpp
    LINKS
        PUBLIC
            Qt6::Core
            Qt6::Widgets
            Qx::Core
)
