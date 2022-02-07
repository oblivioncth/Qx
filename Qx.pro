################# Common Build #################

QT += network
contains(DEFINES, EDITION_WIDGETS) {
    QT += widgets
} else {
    QT -= gui
    CONFIG += console
}

LIB_NAME = Qxtended
LIB_SHORT_NAME = Qx
LIB_VER_MJR = 0
LIB_VER_MNR = 0
LIB_VER_REV = 7
LIB_VER_BLD = 15

# x64 Only
ARCH_STR = 64

VERSION = $${LIB_VER_MJR}.$${LIB_VER_MNR}.$${LIB_VER_REV}.$${LIB_VER_BLD}

contains(DEFINES, EDITION_WIDGETS) {
    CONFIG_STR = W
} else {
    CONFIG_STR = C
}

CONFIG(debug, debug|release) {
  SUFFIX = d
}

TARGET = $${LIB_SHORT_NAME}$${CONFIG_STR}_static$${ARCH_STR}_$${LIB_VER_MJR}-$${LIB_VER_MNR}-$${LIB_VER_REV}-$${LIB_VER_BLD}_Qt_$${QT_MAJOR_VERSION}-$${QT_MINOR_VERSION}-$${QT_PATCH_VERSION}$${SUFFIX}

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++2a

DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x060000

#--------- Edition Common -----------------------
SOURCES += \
    src/qx-io.cpp \
    src/qx-net.cpp \
    src/qx-xml.cpp \
    src/qx.cpp

HEADERS += \
    src/qx-io.h \
    src/qx-net.h \
    src/qx-utility.h \
    src/qx-xml.h \
    src/qx.h

#--------- Edition Console -----------------------
!contains(DEFINES, EDITION_WIDGETS) {

}

#--------- Edition Widgets -----------------------
contains(DEFINES, EDITION_WIDGETS) {
    SOURCES += \
        src/qx-widgets.cpp \

    HEADERS += \
        src/qx-widgets.h \
}

################# Windows Build #################
win32 {
    LIBS += Version.lib
    SOURCES += src/qx-windows.cpp
    HEADERS += src/qx-windows.h
}

################# Linux Build #################
unix:!macx {

}
