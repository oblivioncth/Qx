#================= Add Component ==========================
qx_add_component("Windows-gui" 
    HEADERS_API
        qx-taskbarbutton.h
        qx-winguievent.h
    IMPLEMENTATION
        qx-taskbarbutton.cpp
        qx-winguievent.cpp
        qx-winguieventfilter_p.h
        qx-winguieventfilter.cpp
    LINKS
        PRIVATE
            Qx::Windows
            Qx::Core
        PUBLIC
            ${Qt}::Core
            ${Qt}::Gui
)
