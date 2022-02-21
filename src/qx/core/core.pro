################# Common Build #################
include($$top_src_dir/qx/module_common.pri)

#--------- Edition Common -----------------------
SOURCES += \
    qx-bitarray.cpp \
    qx-char.cpp \
    qx-datetime.cpp \
    qx-genericerror.cpp \
    qx-integrity.cpp \
    qx-json.cpp \
    qx-list.cpp \
    qx-mmrb.cpp \
    qx-string.cpp \
    qx-stringtraverser.cpp

HEADERS += \
    qx-algorithm.h \
    qx-array.h \
    qx-bitarray.h \
    qx-bytearray.h \
    qx-char.h \
    qx-cumulation.h \
    qx-datetime.h \
    qx-endian.h \
    qx-freeindextracker.h \
    qx-genericerror.h \
    qx-index.h \
    qx-integrity.h \
    qx-iostream.h \
    qx-json.h \
    qx-list.h \
    qx-mmrb.h \
    qx-regularexpression.h \
    qx-string.h \
    qx-stringtraverser.h

#--------- Edition Console -----------------------
contains(DEFINES, EDITION_CONSOLE) {

}


#--------- Edition Widgets -----------------------
contains(DEFINES, EDITION_WIDGETS) {

    SOURCES += \
        qx-color.cpp

    HEADERS += \
        qx-color.h
}
