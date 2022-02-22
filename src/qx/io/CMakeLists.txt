#================= Setup ==========================

# Make lib target
qt_add_library(qx-io STATIC)

#================= Build ==========================

################# Common Build #################

#--------- Edition Common -----------------------
# Source Files
target_sources(qx-io
    PRIVATE
        qx-common-io.cpp
        qx-common-io_p.cpp
        qx-filestreamreader.cpp
        qx-filestreamwriter.cpp
        qx-ioopreport.cpp
        qx-textpos.cpp
        qx-textquery.cpp
        qx-textstream.cpp
        qx-textstreamwriter.cpp
)

# Headers
target_sources(qx-io
    PRIVATE
        qx-common-io_p.h
    PUBLIC
        qx-common-io.h
        qx-filestreamreader.h
        qx-filestreamwriter.h
        qx-ioopreport.h
        qx-textpos.h
        qx-textquery.h
        qx-textstream.h
        qx-textstreamwriter.h
)

# Link to Qt libs
target_link_libraries(qx-io PUBLIC Qt6::Core)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()
