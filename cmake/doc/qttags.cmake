# Links Doxygen to Qt6 Docs
# NOTE: Requires the variable QT_DOCS_DIR, which points to the root
#       folder containing the documentation for the target Qt version,
#       to be set

# Define doc modules to link to. Not all of these are necessarily used
set(QT_DOC_MODULES
	qtconcurrent
	qtcore
	qtgui
	qtnetwork
	qtsql
        qtsvg
	qtwidgets
	qtxml
)

# Ensure root exists
if(NOT IS_DIRECTORY ${QT_DOCS_DIR})
        message(FATAL_ERROR "Qt docs path: '${QT_DOCS_DIR}' does not exist!")
endif()

# Link to Qt docs
foreach(doc_module ${QT_DOC_MODULES})
	set(DOXYGEN_TAGFILES ${DOXYGEN_TAGFILES}
                ${QT_DOCS_DIR}/${doc_module}/${doc_module}.tags=https://doc.qt.io/qt-6/
	)
endforeach()
