################# Common Build #################
include($$top_src_dir/qx/module_common.pri)

LIBS += Version.lib

SOURCES += \
    qx-common-windows.cpp \
    qx-filedetails.cpp

HEADERS += \
    qx-common-windows.h \
    qx-filedetails.h

#--------- Edition Console -----------------------
contains(DEFINES, EDITION_CONSOLE) {

}

#--------- Edition Widgets -----------------------
contains(DEFINES, EDITION_WIDGETS) {

}
