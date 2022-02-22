#================= Setup ==========================

# Make lib target
qt_add_library(qx-widgets STATIC)

#================= Build ==========================

################# Common Build #################

#--------- Edition Common -----------------------
# Source Files
target_sources(qx-widgets
    PRIVATE
        qx-logindialog.cpp
        qx-standarditemmodel.cpp
        qx-treeinputdialog.cpp
)

# Headers
target_sources(qx-widgets
    PUBLIC
        qx-logindialog.h
        qx-standarditemmodel.h
        qx-treeinputdialog.h
)


#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")
    # Link to Qt libs
    target_link_libraries(qx-widgets
        PUBLIC
            Qt6::Widgets
            Qt6::Gui
    )
endif()



