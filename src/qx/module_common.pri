################# Common Build #################
CONFIG += c++2a

TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$top_src_dir/qx

DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x060000

#--------- Edition Common -----------------------

#--------- Edition Console -----------------------
contains(DEFINES, EDITION_CONSOLE) {
    QT -= gui
    CONFIG += console
}

#--------- Edition Widgets -----------------------
contains(DEFINES, EDITION_WIDGETS) {
    QT += widgets
}
