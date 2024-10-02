#=== TESTS INTERFACE HELPER LIB ===

# Setup lib target
add_library(${TESTS_COMMON_TARGET} INTERFACE)

target_sources(${TESTS_COMMON_TARGET}
    PRIVATE
        include/qx_test_common.h
)

target_include_directories(${TESTS_COMMON_TARGET}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(${TESTS_COMMON_TARGET}
    INTERFACE
        ${Qt}::Test
)
