#================= Setup ==========================

# Make lib target
qt_add_library(qx-windows STATIC)

################# Common Build #################

#--------- Edition Common -----------------------
# Source Files
target_sources(qx-windows
    PRIVATE
        qx-common-windows.cpp
        qx-filedetails.cpp
)

# Headers
target_sources(qx-windows
    PUBLIC
        qx-common-windows.h
        qx-filedetails.h
)

# Link to Qt libs
target_link_libraries(qx-windows PUBLIC Qt6::Core)

# Link to Windows Version.lib
target_link_libraries(qx-windows PUBLIC Version.lib)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()
