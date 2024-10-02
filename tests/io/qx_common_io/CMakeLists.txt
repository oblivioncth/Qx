include(OB/Test)

ob_add_basic_standard_test(
    TARGET_PREFIX "${TESTS_TARGET_PREFIX}"
    TARGET_VAR test_target
    LINKS
        PRIVATE
            ${TESTS_COMMON_TARGET}
            Qx::Io
)

# Bundle test data
file(GLOB_RECURSE text_data
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "data/*.*"
)

qt_add_resources(${test_target} "tst_qx_common_io_data"
    PREFIX "/"
    FILES
        ${text_data}
)
