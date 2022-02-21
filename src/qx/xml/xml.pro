################# Common Build #################
include($$top_src_dir/qx/module_common.pri)

SOURCES += \
    qx-common-xml.cpp \
    qx-xmlstreamreadererror.cpp \
    qx-xmlstreamwritererror.cpp

HEADERS += \
    qx-common-xml.h \
    qx-xmlstreamreadererror.h \
    qx-xmlstreamwritererror.h

#--------- Edition Console -----------------------
contains(DEFINES, EDITION_CONSOLE) {

}

#--------- Edition Widgets -----------------------
contains(DEFINES, EDITION_WIDGETS) {

}
