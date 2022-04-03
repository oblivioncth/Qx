# Customize Doxygen options
set(DOXYGEN_REPEAT_BRIEF NO)
set(DOXYGEN_WARN_AS_ERROR YES)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_ENABLE_PREPROCESSING YES)
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
set(DOXYGEN_CALL_GRAPH YES)
set(DOXYGEN_CALLER_GRAPH YES)

# Set layout file
set(DOXYGEN_LAYOUT_FILE "${DOC_SOURCE_PATH}/DoxygenLayout.xml")

# Set extra input paths
set(DOXYGEN_EXAMPLE_PATH ${DOC_EXAMPLE_LIST})

# Configure custom command/macro processing
set(DOXYGEN_ALIASES
	[[qflag{2}="@typedef \1^^<p>The \1 type is a typedef for QFlags\<\2\>. It stores an OR combination of \2 values.</p>"]]
	"component{1}=\"@par Import:^^@code find_package(${CMAKE_PROJECT_NAME} REQUIRED COMPONENTS \\1)@endcode ^^@par Link:^^@code target_link_libraries(target_name ${CMAKE_PROJECT_NAME}::\\1)@endcode\""
)

set(DOXYGEN_PREDEFINED
	"Q_DECLARE_FLAGS(flagsType,enumType)=typedef QFlags<enumType> flagsType\;"
)

# Prevent unwanted quoting
set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)

# Link to Qt docs
include(${DOC_SCRIPTS_PATH}/qttags.cmake)

# Set output paths
set(DOXYGEN_OUTPUT_DIRECTORY ${DOC_BUILD_PATH})

