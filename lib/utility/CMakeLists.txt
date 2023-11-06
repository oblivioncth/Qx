#================= Add Component ==========================
qx_add_component("Utility"
    TARGET_VAR utility_target
    HEADERS_API
        qx-concepts.h
        qx-helpers.h
        qx-macros.h
        qx-stringliteral.h
    DOC_ONLY
        qx-concepts.dox
        qx-helpers.dox
        qx-macros.dox
        qx-stringliteral.dox
)

# Force conforming preprocessor for MSVC, required for
# some macros (e.g. QX_FOR_EACH) to work
if(MSVC)
    target_compile_options(${utility_target} INTERFACE "/Zc:preprocessor")
endif()
