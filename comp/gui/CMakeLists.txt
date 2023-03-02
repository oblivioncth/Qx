#================= Add Component ==========================
qx_add_component("Gui"
    HEADERS_API
        qx-color.h
    IMPLEMENTATION
        qx-color.cpp
    LINKS
        PUBLIC
            Qt6::Core
            Qt6::Gui
)
