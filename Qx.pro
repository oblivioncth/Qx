QT += widgets

LIB_NAME = Qxtended
LIB_SHORT_NAME = Qx
LIB_VER_MJR = 0
LIB_VER_MNR = 0
LIB_VER_REV = 1
LIB_VER_BLD = 2

contains(QT_ARCH, i386) {
    ARCH_STR = 32
} else {
    ARCH_STR = 64
}

VERSION = $${LIB_VER_MJR}.$${LIB_VER_MNR}.$${LIB_VER_REV}.$${LIB_VER_BLD}

CONFIG(release, debug|release) {
  TARGET = $${LIB_SHORT_NAME}_static$${ARCH_STR}_$${LIB_VER_MJR}-$${LIB_VER_MNR}-$${LIB_VER_REV}-$${LIB_VER_BLD}_Qt_$${QT_MAJOR_VERSION}-$${QT_MINOR_VERSION}-$${QT_PATCH_VERSION}
}
CONFIG(debug, debug|release) {
  TARGET = $${LIB_SHORT_NAME}_static$${ARCH_STR}_$${LIB_VER_MJR}-$${LIB_VER_MNR}-$${LIB_VER_REV}-$${LIB_VER_BLD}_Qt_$${QT_MAJOR_VERSION}-$${QT_MINOR_VERSION}-$${QT_PATCH_VERSION}d
}

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++1z
QMAKE_CXXFLAGS += /std:c++17

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
    src/qx.cpp

HEADERS += \
    src/Windows_c++17_compat.h \
    src/qx-io.h \
    src/qx.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
