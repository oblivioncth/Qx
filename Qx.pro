QT += network
contains(DEFINES, EDITION_WIDGETS) {
    QT += widgets
} else {
    QT -= gui
    CONFIG += console
}

BUILDS

LIB_NAME = Qxtended
LIB_SHORT_NAME = Qx
LIB_VER_MJR = 0
LIB_VER_MNR = 0
LIB_VER_REV = 7
LIB_VER_BLD = 2

contains(QT_ARCH, i386) {
    ARCH_STR = 32
} else {
    ARCH_STR = 64
}

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

LIBS += Version.lib

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/qx-io.cpp \
    src/qx-net.cpp \
    src/qx-widgets.cpp \
    src/qx-windows.cpp \
    src/qx-xml.cpp \
    src/qx.cpp

HEADERS += \
    src/qx-io.h \
    src/qx-net.h \
    src/qx-widgets.h \
    src/qx-windows.h \
    src/qx-xml.h \
    src/qx.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
