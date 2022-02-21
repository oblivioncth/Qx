################# Common Build #################
include($$top_src_dir/qx/module_common.pri)

#--------- Edition Common -----------------------
QT += network

SOURCES += \
    qx-common-network.cpp \
    qx-networkreplyerror.cpp \
    qx-syncdownloadmanager.cpp

HEADERS += \
    qx-common-network.h \
    qx-networkreplyerror.h \
    qx-syncdownloadmanager.h

#--------- Edition Console -----------------------
contains(DEFINES, EDITION_CONSOLE) {

}

#--------- Edition Widgets -----------------------
contains(DEFINES, EDITION_WIDGETS) {

}
