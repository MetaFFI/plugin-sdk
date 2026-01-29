set(PYTHON_COMPILER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})

# Add Python compiler test
add_py_test(sdk.compiler.python3.host.host_compiler_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Create custom target for Python compiler tests
# Note: The test is added via add_py_test above and will be run by ctest
# This target exists for consistency with other compiler test targets
add_custom_target(compiler_python3_tests)

# Bubble up target to parent scope
set(compiler_python3_tests compiler_python3_tests PARENT_SCOPE)
