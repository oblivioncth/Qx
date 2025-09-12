# Include standard documentation target creator
include(OB/Documentation)

# Gather component level input
foreach(component ${LIB_COMPONENTS})
    list(APPEND COMPONENT_DOC_ROOTS "${COMPONENTS_PATH}/${component}/doc")
    list(APPEND COMPONENT_DOC_INPUT "${COMPONENTS_PATH}/${component}/src")
    list(APPEND COMPONENT_DOC_INPUT "${COMPONENTS_PATH}/${component}/include")
endforeach()

# Set project specific doxygen overrides
set(DOXYGEN_PREDEFINED
    "QX_ERROR_TYPE(Type,Name,Code)=Type final : public Qx::AbstractError<Name, Code>"
    "__QX_JSON_META_STRUCT="
    "QX_DECLARE_ERROR_ADAPTATION(Adaptable, Adapter)="
)

set(DOXYGEN_EXCLUDE_SYMBOLS
    "_QxPrivate"
    "__QX*"
)

include(cmake/SqlMacros.cmake)

# Setup documentation
ob_standard_documentation(${DOC_TARGET_NAME}
    DOXY_VER 1.9.4
    PROJ_NAME "${PROJECT_NAME}"
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
    THEME_COLORS
        PRIMARY "#CC2828"
        NIGHT_PRIMARY "#E22D2D"
)
