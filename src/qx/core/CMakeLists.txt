#================= Setup ==========================

# Make lib target
qt_add_library(qx-core STATIC)

#================= Build ==========================

################# Common Build #################

#--------- Edition Common -----------------------
# Source Files
target_sources(qx-core
    PRIVATE
        qx-bitarray.cpp
        qx-char.cpp
        qx-datetime.cpp
        qx-genericerror.cpp
        qx-integrity.cpp
        qx-json.cpp
        qx-list.cpp
        qx-mmrb.cpp
        qx-string.cpp
        qx-stringtraverser.cpp
)

# Headers
target_sources(qx-core
    PUBLIC
        qx-algorithm.h
        qx-array.h
        qx-bitarray.h
        qx-bytearray.h
        qx-char.h
        qx-cumulation.h
        qx-datetime.h
        qx-endian.h
        qx-freeindextracker.h
        qx-genericerror.h
        qx-index.h
        qx-integrity.h
        qx-iostream.h
        qx-json.h
        qx-list.h
        qx-mmrb.h
        qx-regularexpression.h
        qx-string.h
        qx-stringtraverser.h
)


# Link to Qt libs
target_link_libraries(qx-core PUBLIC Qt6::Core)

#--------- Edition Console -----------------------
if(EDITION STREQUAL "console")

endif()

#--------- Edition Widgets -----------------------
if(EDITION STREQUAL "widgets")
    # Source Files
    target_sources(qx-core
        PRIVATE
            qx-color.cpp
    )

    # Headers
    target_sources(qx-core
        PUBLIC
            qx-color.h
    )

    # Link to Qt libs
    target_link_libraries(qx-core
        PUBLIC
            Qt6::Widgets
            Qt6::Gui
    )
endif()
