#================= Setup ==========================

# Make lib target
qt_add_library(qx-network STATIC)

#================= Build ==========================

################# Common Build #################

#--------- Edition Common -----------------------
# Source Files
target_sources(qx-network
    PRIVATE
        qx-common-network.cpp
        qx-networkreplyerror.cpp
        qx-syncdownloadmanager.cpp
)

# Headers
target_sources(qx-network
    PUBLIC
        qx-common-network.h
        qx-networkreplyerror.h
        qx-syncdownloadmanager.h
)

# Link to Qt libs
target_link_libraries(qx-network
    PUBLIC
        Qt6::Core
        Qt6::Network
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()
