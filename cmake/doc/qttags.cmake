# Links Doxygen to Qt6 Docs
# NOTE: Requires the variable QT_DOCS_DIR, which points to the root
#       folder containing the documentation for one or more Qt versions
#       to be set

# Define doc modules to link to. Not all of these are necessarily used
set(QT_DOC_MODULES
	qtconcurrent
	qtcore
	qtgui
	qtnetwork
	qtsql
	qtwidgets
	qtxml
)

# Determine base doc directory for the imported Qt version
set(QT_DOC_TAGS_ROOT "${QT_DOCS_DIR}/Qt-${Qt6_VERSION}")

# Ensure root exists
if(NOT IS_DIRECTORY ${QT_DOC_TAGS_ROOT})
	message(FATAL_ERROR "Docs for Qt6: ${Qt6_VERSION} could not be located!")
endif()

# Link to Qt docs
foreach(doc_module ${QT_DOC_MODULES})
	set(DOXYGEN_TAGFILES ${DOXYGEN_TAGFILES}
		${QT_DOC_TAGS_ROOT}/${doc_module}/${doc_module}.tags=https://doc.qt.io/qt-6/
	)
endforeach()
