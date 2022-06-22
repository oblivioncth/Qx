function(configure_qt_doc_link qt_prefix)
    # Handle using cache so that users can easily override via UI or command-line

    # Could use crazy amounts of file system searching to check for every Qt root under the standard Qt install
    # location for each platform (i.e. C:/Program Files/Qt/6.3.0/msvc2019/) and then add those as PATHS to the
    # below commands, but for now, lets not

    # Locate qhelpgenerator
    find_file(QT_HELP_GEN_PATH
        NAMES
            "qhelpgenerator"
            "qhelpgenerator.exe"
        HINTS
            "${qt_prefix}"
        PATH_SUFFIXES
            "bin"
            "libexec"
        DOC "Path to the qhelpgenerator executable"
        NO_DEFAULT_PATH
    )

    if(QT_HELP_GEN_PATH)
        message(STATUS "qhelpgenerator found at: ${QT_HELP_GEN_PATH}")
    else()
        message(WARNING "Could not find qhelpgenerator. Please set QT_HELP_GEN_PATH to its location if you want to generate a .qch file for documentation.")
    endif()

    # Locate Qt documentation
    find_path(QT_DOCS_DIR
        NAMES
            "qtcore.qch"
            "qtcore"
        HINTS
            "${qt_prefix}/doc"
        PATHS
            "C:/Program Files/Qt/Docs/Qt-${Qt6_VERSION}"
        DOC "Path to Qt documentation"
        NO_DEFAULT_PATH
    )

    if(QT_DOCS_DIR)
        message(STATUS "Qt documentation found at: ${QT_DOCS_DIR}")
    else()
        message(WARNING "Could not find documentation for the in-use Qt version. Please set QT_DOCS_DIR to its location if you want the generated documentation to reference Qt.")
    endif()
endfunction()
