################# Common Build #################
include($$top_src_dir/qx/module_common.pri)

#--------- Edition Common -----------------------
SOURCES += \
    qx-common-io.cpp \
    qx-common-io_p.cpp \
    qx-filestreamreader.cpp \
    qx-filestreamwriter.cpp \
    qx-ioopreport.cpp \
    qx-textpos.cpp \
    qx-textquery.cpp \
    qx-textstream.cpp \
    qx-textstreamwriter.cpp


HEADERS += \
    qx-common-io.h \
    qx-common-io_p.h \
    qx-filestreamreader.h \
    qx-filestreamwriter.h \
    qx-ioopreport.h \
    qx-textpos.h \
    qx-textquery.h \
    qx-textstream.h \
    qx-textstreamwriter.h

#--------- Edition Console -----------------------
contains(DEFINES, EDITION_CONSOLE) {

}

#--------- Edition Widgets -----------------------
contains(DEFINES, EDITION_WIDGETS) {

}
