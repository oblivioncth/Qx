#================= Setup Component ==========================
set(COMPONENT_NAME "Io")

qx_get_component_qt_depends("${COMPONENT_NAME}" COMPONENT_QT_DEPENDS)
qx_get_component_sibling_depends("${COMPONENT_NAME}" COMPONENT_SIBLING_DEPENDS)

qx_register_component(
    NAME "${COMPONENT_NAME}"
    DEPENDS_QT ${COMPONENT_QT_DEPENDS}
    DEPENDS_SIBLINGS ${COMPONENT_SIBLING_DEPENDS}
    HEADERS
        PRIVATE
            qx-common-io_p.h
        PUBLIC
            qx-applicationlogger.h
            qx-common-io.h
            qx-filestreamreader.h
            qx-filestreamwriter.h
            qx-ioopreport.h
            qx-textpos.h
            qx-textquery.h
            qx-textstream.h
            qx-textstreamreader.h
            qx-textstreamwriter.h
    IMPLEMENTATION
        qx-applicationlogger.cpp
        qx-common-io.cpp
        qx-common-io_p.cpp
        qx-filestreamreader.cpp
        qx-filestreamwriter.cpp
        qx-ioopreport.cpp
        qx-textpos.cpp
        qx-textquery.cpp
        qx-textstream.cpp
        qx-textstreamreader.cpp
        qx-textstreamwriter.cpp
    LINKS
        PUBLIC
            Qt::Core
            Qx::Core
)
