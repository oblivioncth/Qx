#================= Setup ==========================

# Make lib target
qt_add_library(qx-xml STATIC)

#================= Build ==========================

################# Common Build #################

#--------- Edition Common -----------------------
# Source Files
target_sources(qx-xml
    PRIVATE
        qx-common-xml.cpp
        qx-xmlstreamreadererror.cpp
        qx-xmlstreamwritererror.cpp
)

# Headers
target_sources(qx-xml
    PUBLIC
        qx-common-xml.h
        qx-xmlstreamreadererror.h
        qx-xmlstreamwritererror.h
)

# Link to Qt libs
target_link_libraries(qx-xml
    PUBLIC
        Qt6::Core
        Qt6::Xml
)

#--------- Edition Console -----------------------

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()
