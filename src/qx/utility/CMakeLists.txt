#================= Setup ==========================

# Make lib target
qt_add_library(qx-utility INTERFACE)

#================= Build ==========================

################# Common Build #################

#--------- Edition Common -----------------------
# Headers
target_sources(qx-utility
    PUBLIC
        qx-concepts.h
        qx-helpers.h
        qx-macros.h
)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")

endif()
