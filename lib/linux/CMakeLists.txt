#================= Add Component ==========================
qx_add_component("Linux"
    HEADERS_API
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
