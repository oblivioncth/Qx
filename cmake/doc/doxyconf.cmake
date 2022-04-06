#-----------Main Doxygen Options-----------------------------

# General
set(DOXYGEN_REPEAT_BRIEF NO)
set(DOXYGEN_WARN_AS_ERROR YES)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_ENABLE_PREPROCESSING YES)
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
set(DOXYGEN_GROUP_NESTED_COMPOUND YES)
set(DOXYGEN_ENUM_VALUES_PER_LINE 1)
set(DOXYGEN_EXT_LINKS_IN_WINDOW YES)

# Set logo
set(DOXYGEN_PROJECT_LOGO "${DOC_SOURCE_PATH}/logo.svg")

# Add 'v' prefix to version number
set(DOXYGEN_PROJECT_NUMBER v${CMAKE_PROJECT_VERSION})

# Set layout file
set(DOXYGEN_LAYOUT_FILE "${DOC_SOURCE_PATH}/DoxygenLayout.xml")

# Set extra input paths
set(DOXYGEN_EXAMPLE_PATH ${DOC_EXAMPLE_LIST})
set(DOXYGEN_IMAGE_PATH ${DOC_IMAGE_LIST})

# Configure custom command/macro processing
set(DOXYGEN_ALIASES
	[[qflag{2}="@typedef \1^^<p>The \1 type is a typedef for QFlags\<\2\>. It stores an OR combination of \2 values.</p>"]]
        "component{2}=\"@par Import:^^@code find_package(${CMAKE_PROJECT_NAME} REQUIRED COMPONENTS \\1)@endcode ^^@par Link:^^@code target_link_libraries(target_name ${CMAKE_PROJECT_NAME}::\\1)@endcode ^^@par Include:^^@code #include <${PROJ_NAME_LC}/\\2>@endcode\""
)

set(DOXYGEN_PREDEFINED
	"Q_DECLARE_FLAGS(flagsType,enumType)=typedef QFlags<enumType> flagsType\;"
)

# Prevent unwanted quoting
set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)

# Link to Qt docs
if(DEFINED QT_DOCS_DIR)
    include(${DOC_SCRIPTS_PATH}/qttags.cmake)
else()
    message(WARNING "QT_DOCS_DIR is not defined, Qt Documentation will not be linked to.")
endif()

# Setup Qt Creator Help File
if(DEFINED QT_HELP_GEN_PATH)
    set(DOXYGEN_GENERATE_QHP YES)
    set(DOXYGEN_QCH_FILE "../${CMAKE_PROJECT_NAME}.qch")
    set(DOXYGEN_QHG_LOCATION ${QT_HELP_GEN_PATH})
else()
    message(WARNING "QT_HELP_GEN_PATH is not defined, a .qch file will not be generated.")
endif()

# Set output paths
set(DOXYGEN_OUTPUT_DIRECTORY ${DOC_BUILD_PATH})


#-------------Doxygen Awsome Options--------------

# Base Theme
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_HTML_EXTRA_STYLESHEET ${DOXYGEN_HTML_EXTRA_STYLESHEET}
    "${DOC_SOURCE_PATH}/theme/doxygen-awesome/doxygen-awesome.css"
    "${DOC_SOURCE_PATH}/theme/doxygen-awesome/doxygen-awesome-sidebar-only.css"
)

# Customization
set(DOXYGEN_HTML_EXTRA_STYLESHEET ${DOXYGEN_HTML_EXTRA_STYLESHEET}
    "${DOC_SOURCE_PATH}/theme/doxygen-awesome/doxygen-awesome-customize.css"
)
set(DOXYGEN_HTML_COLORSTYLE_HUE 0)
set(DOXYGEN_HTML_COLORSTYLE_SAT 255)
set(DOXYGEN_HTML_COLORSTYLE_GAMMA 80)

# Extensions - General
set(DOXYGEN_HTML_HEADER "${DOC_SOURCE_PATH}/header.html")

# Extensions - Dark Mode Toggle
set(DOXYGEN_HTML_EXTRA_FILES ${DOXYGEN_HTML_EXTRA_FILES}
    "${DOC_SOURCE_PATH}/theme/doxygen-awesome/doxygen-awesome-darkmode-toggle.js"
)
set(DOXYGEN_HTML_EXTRA_STYLESHEET ${DOXYGEN_HTML_EXTRA_STYLESHEET}
    "${DOC_SOURCE_PATH}/theme/doxygen-awesome/doxygen-awesome-sidebar-only-darkmode-toggle.css"
)

# Extensions - Fragment Copy Button
set(DOXYGEN_HTML_EXTRA_FILES ${DOXYGEN_HTML_EXTRA_FILES}
    "${DOC_SOURCE_PATH}/theme/doxygen-awesome/doxygen-awesome-fragment-copy-button.js"
)

# Extensions - Paragraph Linking
set(DOXYGEN_HTML_EXTRA_FILES ${DOXYGEN_HTML_EXTRA_FILES}
    "${DOC_SOURCE_PATH}/theme/doxygen-awesome/doxygen-awesome-paragraph-link.js"
)

# Best matching class diagram options
set(DOXYGEN_DOT_IMAGE_FORMAT svg)
set(DOXYGEN_DOT_TRANSPARENT YES)
