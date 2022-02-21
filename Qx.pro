################# Common Build #################
TEMPLATE = subdirs

#--------- Edition Common -----------------------
SUBDIRS = src/qx/core \
          src/qx/io \
          src/qx/network \
          src/qx/xml

include(src/qx/utility/utility.pri)

#--------- Edition Widgets ----------------------
contains(DEFINES, EDITION_WIDGETS) {
    SUBDIRS += src/qx/widgets
}

#--------- Edition Console ----------------------
contains(DEFINES, EDITION_CONSOLE) {

}

################# Windows Build #################
win32 {
	SUBDIRS += src/qx/windows
}

################# Linux Build #################
unix:!macx {

}
