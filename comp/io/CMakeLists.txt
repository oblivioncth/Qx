#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(COMPONENT_INCLUDE_HEADERS
    qx-common-io.h
    qx-filestreamreader.h
    qx-filestreamwriter.h
    qx-ioopreport.h
    qx-textpos.h
    qx-textquery.h
    qx-textstream.h
    qx-textstreamwriter.h
)

set(COMPONENT_PRIVATE_HEADERS
    qx-common-io_p.h
)

# Source Files
set(COMPONENT_IMPL
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

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt::Core
    Qx::Core
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()

#=============== Component Handler =======================
include(component_lib_rules)

