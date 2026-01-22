
# Add Go test target
add_go_test(cdts_go_serializer_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# Note: Go serializer is a library, not a shared library
# It will be used by Go code that imports it
# No build target needed - Go handles compilation via go.mod
