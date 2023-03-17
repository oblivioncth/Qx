# Include standard documentation target creator
include(OB/Documentation)

# Gather component level input
foreach(component ${LIB_COMPONENTS})
    list(APPEND COMPONENT_DOC_ROOTS "${COMPONENTS_PATH}/${component}/doc")
    list(APPEND COMPONENT_DOC_INPUT "${COMPONENTS_PATH}/${component}/src")
    list(APPEND COMPONENT_DOC_INPUT "${COMPONENTS_PATH}/${component}/include")
endforeach()

# Set project specific doxygen overrides
set(DOXYGEN_HTML_COLORSTYLE_HUE 0)
set(DOXYGEN_HTML_COLORSTYLE_SAT 255)
set(DOXYGEN_HTML_COLORSTYLE_GAMMA 80)

# Setup documentation
ob_standard_documentation(${DOC_TARGET_NAME}
    DOXY_VER 1.9.4
    PROJ_NAME "${PROJECT_NAME}"
    QT_PREFIX "${Qt6_PREFIX_PATH}"
    QT_MODULES
        qtconcurrent
        qtcore
        qtgui
        qtnetwork
        qtsql
        qtsvg
        qtwidgets
        qtxml
    ADDITIONAL_ROOTS
        ${COMPONENT_DOC_ROOTS}
    INPUT_LIST
        ${COMPONENT_DOC_INPUT}
)
