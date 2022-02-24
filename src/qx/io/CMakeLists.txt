#================= Setup Module ==========================

set(MODULE_TYPE STATIC)

################# Common Build #################

#--------- Edition Common -----------------------

# Headers
set(MODULE_PUBLIC_HEADERS
    qx-common-io.h
    qx-filestreamreader.h
    qx-filestreamwriter.h
    qx-ioopreport.h
    qx-textpos.h
    qx-textquery.h
    qx-textstream.h
    qx-textstreamwriter.h
)

set(MODULE_PRIVATE_HEADERS
    qx-common-io_p.h
)

# Source Files
set(MODULE_IMPL
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
set(MODULE_PUBLIC_LINKS
    Qt::Core
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()

#=============== Module Handler =======================
include(lib_module_rules)

